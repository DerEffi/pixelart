import DataService from '../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { FileUpload } from 'primereact/fileupload';
import {Buffer} from 'buffer';
import { Tooltip } from 'primereact/tooltip';
import { Button } from 'primereact/button';
import {Md5} from 'ts-md5';
import { asyncTimeout, c2dArray, cAnimationArray, rgb565, rgb888 } from '../../services/Helper';
import { InputText } from 'primereact/inputtext';
import { Status } from '../../models/Status';
import JSZip from 'jszip';
import { GifReader } from 'omggif';
import { Knob } from 'primereact/knob';
import { ProgressSpinner } from 'primereact/progressspinner';
import { ICachedImage } from '../../models/Cache';
import { Dialog } from 'primereact/dialog';
import moment from 'moment';
import { ConfirmPopup, confirmPopup } from 'primereact/confirmpopup';

export interface IGeneratorComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IGeneratorComponentState {
	canvasImages: (HTMLImageElement | ImageData)[];
	name: string;
	frameTime: number;
	frameInterval: NodeJS.Timer | null;
	rendering: boolean;
	cachedItems: ICachedImage[];
	viewHistory: boolean;
	showTmpCopy: boolean;
}

export default class Generator extends React.Component<IGeneratorComponentProps, IGeneratorComponentState> {

	private currentFrame: number = 0;

	constructor(props: IGeneratorComponentProps) {
		super(props);

		this.state = {
			canvasImages: [],
			name: "",
			frameTime: 100,
			frameInterval: null,
			rendering: false,
			cachedItems: [],
			viewHistory: false,
			showTmpCopy: false
		}
	}

	componentDidMount(): void {
		this.loadCache();
	}

	componentWillUnmount(): void {
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">

					<Dialog header="Image History" closable closeOnEscape dismissableMask draggable={false} resizable={false} maximizable={false} visible={this.state.viewHistory} className="image-generator-history content" headerClassName="image-generator-history-header" onHide={() => this.setState({viewHistory: false})}>
						<div className='image-generator-history-images'>
							<Tooltip target={".custom-upload-history-item"} content="Draw Image" position="bottom" />
							
							
							{this.state.cachedItems.map(image => {
								return(
									<div key={image.id} className='image-generator-history-image'>
										<Tooltip target={".custom-upload-history-item-" + image.id} position="bottom">
											<div className='image-generator-history-tooltip'>
												<div>{image.name}</div>
												<div>{moment.unix(image.created).format("DD.MM.YYYY HH:mm")}</div>
												<div>animated {image.animation.length > 0 ? <i className="pi pi-check"></i> : <i className="pi pi-times"></i>}</div>
											</div>
										</Tooltip>
										<div className="image-generator-item image-generator-history-item">
											<img className="image-generator-item-image" alt={image.name} role="presentation" src={image.preview} width={100} height={100} />
											<div className="image-generator-item-buttons">

												<Button type="button" icon="pi pi-pencil" className={"custom-upload-history-item p-button-outlined p-button-rounded p-button-success ml-auto"} onClick={() => this.loadFromCache(image.name, image.image, image.animation)}/>
												<Button type="button" icon="pi pi-info" className={"custom-upload-history-item-" + image.id + " p-button-outlined p-button-rounded p-button-info ml-auto"} />
											</div>
										</div>
									</div>
								);
							})}
						</div>
					</Dialog>
				
					<div id="image-generator-canvas-wrapper">
						{this.state.rendering &&
							<>
								<div className='image-generator-rendering'>
									<div style={{zIndex: 1}}>
										<ProgressSpinner animationDuration="2s" strokeWidth='3' />
										<div>Processing</div>
									</div>
									<div className='image-generator-rendering-blocker'/>
								</div>
							</>
						}
						<canvas id="image-generator-canvas" width={64} height={64} />
					</div>

					<div className="image-generator-menu input-group" style={{marginTop: "1rem"}}>
						<Tooltip target=".image-generator-menu-button" position="bottom" />

						<div className="input-group-field p-inputgroup">
    						<span className="p-inputgroup-addon">xxx - </span>
							<InputText placeholder='image name' value={this.state.name} onChange={(e) => this.setState({name: e.target.value})} />
						</div>
						<Button type="button" disabled={this.state.rendering} icon="pi pi-download" data-pr-tooltip="Download Pixelart" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.downloadImage()}/>
						<Button type="button" icon="pi pi-upload" data-pr-tooltip="Upload to Device" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.uploadToDevice()} disabled={this.props.dataService.getStatus() !== Status.connected || this.state.rendering}/>
					</div>

					{this.props.advanced &&
						<div className="input-group" style={{marginTop: "1rem"}}>
							<Tooltip target=".image-generator-menu-button-advanced" position="bottom" />
							<ConfirmPopup />

							<Button type="button" icon={"pi pi-" + (this.state.frameInterval ? "pause" : "play")} data-pr-tooltip="Play/Pause" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.playAnimation()}/>
							<Button type="button" icon="pi pi-step-forward" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-success ml-auto" data-pr-tooltip="Next Frame" disabled={this.state.frameInterval != null} onClick={() => this.drawFrame()} />
							<Knob className="image-generator-menu-button-advanced" data-pr-tooltip="Frametime (ms)" size={65} value={this.state.frameTime} step={25} min={25} max={500} onChange={(e) => this.adjustSpeed(e.value)} />
							<Button type="button" icon="pi pi-copy" data-pr-tooltip="Copy hex" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-help ml-auto" onClick={(e) => this.copyByteArray(e)}/>
							<Button type="button" icon="pi pi-code" data-pr-tooltip="Copy source code" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-help ml-auto" onClick={(e) => this.copyCArray(e)}/>
							<Button type="button" icon="pi pi-trash" data-pr-tooltip="Clear Image" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-error ml-auto" onClick={() => this.clearCanvas()}/>
						</div>
					}

					<div style={{marginTop: "1.5rem"}}>
						<Tooltip target=".custom-choose-btn" content="Open Files" position="bottom" />
						<Tooltip target=".custom-upload-btn" content="Draw Animation from Images" position="bottom" />
						<Tooltip target=".custom-cancel-btn" content="Remove Images" position="bottom" />

						<FileUpload
							multiple
							accept="image/*"
							headerTemplate={(options) => {
								const { className, chooseButton, uploadButton, cancelButton } = options;
							
								return (
									<div className={className} style={{ backgroundColor: 'transparent', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
										<div style={{ backgroundColor: 'transparent', display: 'flex', alignItems: 'center' }}>
											{chooseButton}
											{uploadButton}
											{cancelButton}
										</div>
										<div>
											<Button type="button" disabled={this.state.cachedItems.length <= 0 || this.state.rendering} icon="pi pi-history" data-pr-tooltip="Last Images" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-help ml-auto" onClick={() => this.setState({viewHistory: true})}/>
										</div>
									</div>
								);
							}}
							disabled={this.state.rendering}
							itemTemplate={(file: any, props) => {
								let md5name = Md5.hashStr(file.name);
								return(
									<div className="image-generator-item">
										<img className="image-generator-item-image" alt={file.name} role="presentation" src={file.objectURL} width={100} height={100} />
										<div className="image-generator-item-buttons">
											<Tooltip target={".custom-upload-item-" + md5name} content="Draw Image" position="bottom" />
											<Tooltip target={".custom-cancel-item-" + md5name} content="Remove" position="bottom" />

											<Button type="button" disabled={this.state.rendering} icon="pi pi-pencil" className={"custom-upload-item-" + md5name + " p-button-outlined p-button-rounded p-button-success ml-auto"} onClick={() => this.uploadImages([file])}/>
											<Button type="button" disabled={this.state.rendering} icon="pi pi-times" className={"custom-cancel-item-" + md5name + " p-button-outlined p-button-rounded p-button-danger ml-auto"} onClick={props.onRemove}/>
										</div>
									</div>
								)
							}}
							emptyTemplate={
								<div style={{textAlign: "center"}}>
									<div style={{display: "inline-block", textAlign: "center"}}>
										<i className="pi pi-image mt-3 p-5" style={{ fontSize: '3.5rem', color: 'var(--surface-600)' }}></i>
										<br/>
										<span style={{ fontSize: '1.2rem', color: 'var(--text-color-secondary)' }}>
											Drag and Drop Images Here
										</span>
									</div>
								</div>
							}
							progressBarTemplate={<></>}
							chooseOptions={{ icon: 'pi pi-fw pi-folder-open', iconOnly: true, className: 'custom-choose-btn p-button-rounded p-button-info p-button-outlined', label: "Select" }}
							uploadOptions={{ icon: 'pi pi-fw pi-images', iconOnly: true, className: 'custom-upload-btn p-button-success p-button-rounded p-button-outlined', label: "Generate as Animation" }}
							cancelOptions={{ icon: 'pi pi-fw pi-times', iconOnly: true, className: 'custom-cancel-btn p-button-danger p-button-rounded p-button-outlined', label: "Clear" }}
							customUpload
							uploadHandler={(e) => this.uploadImages(e.files)}
							contentClassName="image-generator-upload"
						/>
					</div>

				</div>
				
            </div>
        );
    }

	//Prepare image data for preview canvas from selected files
	private async uploadImages(files: File[]) {
		this.state.canvasImages.forEach(img => {
			if(img instanceof HTMLImageElement)
				img.remove();
		});
		this.setState({
			canvasImages: [],
		}, async () => {
			if(files.length > 0) {
				let images: (HTMLImageElement | ImageData)[] = [];

				for(let i: number = 0; i < files.length; i++) {
					if(!files[i].type.startsWith("image/")) {
						if(this.props.toast)
							this.props.toast.show({
								content: "Please select proper image",
								severity: 'error',
								closable: false
							});
						continue;
					} else if (files[i].type === "image/gif") {
						let gifReader = new GifReader(new Uint8Array(await files[i].arrayBuffer()));
						
						for(let frame: number = 0; frame < gifReader.numFrames(); frame++) {
							let frameData = new ImageData(64, 64);
							gifReader.decodeAndBlitFrameRGBA(frame, frameData.data as any);
							images.push(frameData);
						}
					} else {
						let byteArray = new Uint8Array(await files[i].arrayBuffer());
						let base64image = Buffer.from(byteArray).toString("base64");
			
						let img = document.createElement("img");
						img.src = "data:" + files[i].type + ";base64, " + base64image;
						img.width = 64;
						img.height = 64;
						img.alt = "Hidden container for generating canvas content";
						img.className = "image-generator-image";

						images.push(img);
					}
				}

				this.setState({
					canvasImages: images,
					name: files[0].name.substring(0, files[0].name.lastIndexOf(".") !== -1 ? files[0].name.lastIndexOf(".") : files[0].name.length)
				}, () => setTimeout(() => this.drawFromImages(), 50));
			} else {
				if(this.props.toast)
					this.props.toast.show({
						content: "No image selected",
						severity: 'error',
						closable: false
					});
			}
		});
	}

	//start draw of first frame of prepared images and start animation if prepared image has one
	private drawFromImages() {
		this.currentFrame = 0;
		this.drawFrame();
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
		this.setState({frameInterval: this.state.canvasImages.length > 1 ? setInterval(() => this.drawFrame(), this.state.frameTime) : null});
	}

	//draw selected frame on selected canvas - stop animation if no animation data is present
	private drawFrame(frame: number = ++this.currentFrame, canvas: CanvasRenderingContext2D = this.getCanvas()) {
		if(this.state.canvasImages.length > 0) {
			frame = frame % this.state.canvasImages.length;
			let frameData = this.state.canvasImages[frame];
			if(frameData instanceof HTMLImageElement)
				canvas.drawImage(frameData, 0, 0, 64, 64);
			else if(frameData instanceof ImageData)
				canvas.putImageData(frameData, 0, 0);
			this.currentFrame = frame;

			if(this.state.canvasImages.length === 1 && this.state.frameInterval) {
				clearInterval(this.state.frameInterval);
				this.setState({frameInterval: null});
			}
		} else {
			if(this.state.frameInterval) {
				clearInterval(this.state.frameInterval);
				this.setState({
					frameInterval: null
				});
			}
			if(this.props.toast)
				this.props.toast.show({
					content: "No image to generate pixel art from",
					severity: 'error',
					closable: false
				});
		}
	}

	//start / stop animation
	private playAnimation() {
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
		this.setState({
			frameInterval: this.state.frameInterval || this.state.canvasImages.length <= 1 ? null : setInterval(() => this.drawFrame(), this.state.frameTime)
		});
	}

	//set frametime for animation
	private adjustSpeed(time: number) {
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
		this.setState({
			frameTime: time,
			frameInterval: this.state.frameInterval ? setInterval(() => this.drawFrame(), time) : null
		});
	}

	//calculate image and animation data for prepared images to download as zip
	private async downloadImage() {
		this.setState({rendering: true});
		if(this.state.frameInterval)
			this.playAnimation();

		let ctx: CanvasRenderingContext2D | null = this.newCanvas();
		if(ctx && this.state.canvasImages.length > 0) {
			this.drawFrame(0, ctx);

			let preview: string = ctx.canvas.toDataURL();
			
			let animation: number[][][] = [];

			if(this.state.canvasImages.length > 1) {
				for(let i = 1; i < this.state.canvasImages.length; i++) {
					animation.push(await this.getPixelChanges(i - 1, i));
					await asyncTimeout(1);
				}

				//add loopback frame
				animation.push(await this.getPixelChanges(this.state.canvasImages.length - 1, 0));
				await asyncTimeout(1);
			}

			let image = this.getPixelArray(ctx);
			let frames = animation.length > 0 ? this.getAnimationArray(animation) : [];

			let zip = new JSZip();
			zip.file("000 - " + this.state.name + "/image.pxart", image);
			zip.file("place inside images folder on your sd card", "");
			if(animation.length)
				zip.file("000 - " + this.state.name + "/animation.pxart", frames);
			zip.generateAsync({type: "blob"}).then((blob: Blob) => {
				let file = document.createElement("a");
				file.href = window.URL.createObjectURL(blob);
				file.download = this.state.name;
				file.click();
				file.remove();
			});

			ctx.canvas.remove();

			this.cacheImage(this.state.name, image, animation, preview);
		} else {
			if(this.props.toast)
				this.props.toast.show({
					content: "No image to generate pixel art from",
					severity: 'error',
					closable: false
				});
		}

		if(this.state.canvasImages.length > 1)
			this.playAnimation();

			this.setState({rendering: false});
	}

	//calculate image and animation data for prepared images to directly upload to connected device
	private uploadToDevice() {
		//TODO
	}

	//copy image and animation data from current preview for use in c/c++
	private async copyCArray(event: React.MouseEvent<HTMLButtonElement | MouseEvent>) {
		if(this.state.frameInterval) {
			this.setState({rendering: true});
			this.playAnimation();

			let ctx: CanvasRenderingContext2D | null = this.newCanvas();
			if(ctx && this.state.canvasImages.length > 0) {
				this.drawFrame(0, ctx);
				
				let animation: number[][][] = [];

				for(let i = 1; i < this.state.canvasImages.length; i++) {
					animation.push(await this.getPixelChanges(i - 1, i));
					await asyncTimeout(1);
				}

				//add loopback frame
				animation.push(await this.getPixelChanges(this.state.canvasImages.length - 1, 0));
				await asyncTimeout(1);
				ctx.canvas.remove();

				let maxPixelChanges = 0;
				animation.forEach(frame => maxPixelChanges = frame.length > maxPixelChanges ? frame.length : maxPixelChanges);
				(window as any).tmpPixelData = "uint16_t image[64][64] = " + c2dArray(this.get2dPixelArray()) + ";\nuint16 animation[" + animation.length + "][" + maxPixelChanges + "][3] = " + cAnimationArray(animation) + ";";
				confirmPopup({
					target: event.target as HTMLElement,
					message: "Your data is now ready",
					acceptLabel: "Copy",
					acceptIcon: "pi pi-copy",
					accept: () => navigator.clipboard.writeText((window as any).tmpPixelData),
					rejectLabel: ""
				});
			} else {
				if(this.props.toast)
					this.props.toast.show({
						content: "No image to generate pixel art from",
						severity: 'error',
						closable: false
					});
			}

			if(this.state.canvasImages.length > 1)
				this.playAnimation();

				this.setState({rendering: false});
		} else {
			navigator.clipboard.writeText(c2dArray(this.get2dPixelArray()));
		}
	}

	//copy base64 binary data from current preview for hex editor
	private async copyByteArray(event: React.MouseEvent<HTMLButtonElement | MouseEvent>) {
		if(this.state.frameInterval) {
			this.setState({rendering: true});
			if(this.state.frameInterval)
				this.playAnimation();

			let ctx: CanvasRenderingContext2D | null = this.newCanvas();
			if(ctx && this.state.canvasImages.length > 0) {
				this.drawFrame(0, ctx);
				
				let animation: number[][][] = [];

				if(this.state.canvasImages.length > 1) {
					for(let i = 1; i < this.state.canvasImages.length; i++) {
						animation.push(await this.getPixelChanges(i - 1, i));
						await asyncTimeout(1);
					}

					//add loopback frame
					animation.push(await this.getPixelChanges(this.state.canvasImages.length - 1, 0));
					await asyncTimeout(1);
				}

				let frames = animation.length > 0 ? this.getAnimationArray(animation) : [];

				ctx.canvas.remove();

				(window as any).tmpImageData = Buffer.from(this.getPixelArray()).toString("base64");
				(window as any).tmpAnimationData = Buffer.from(frames).toString("base64");
				
				confirmPopup({
					target: event.target as HTMLElement,
					message: "Your data is now ready",
					rejectLabel: "Copy Image",
					rejectIcon: "pi pi-copy",
					reject: () => navigator.clipboard.writeText((window as any).tmpImageData),
					acceptLabel: frames.length > 0 ? "Copy Animation" : " ",
					acceptIcon: frames.length > 0 ? "pi pi-copy" : "",
					accept: () => navigator.clipboard.writeText((window as any).tmpAnimationData),
				});
			} else {
				if(this.props.toast)
					this.props.toast.show({
						content: "No image to generate pixel art from",
						severity: 'error',
						closable: false
					});
			}

			if(this.state.canvasImages.length > 1)
				this.playAnimation();

				this.setState({rendering: false});
		} else {
			navigator.clipboard.writeText(Buffer.from(this.getPixelArray()).toString("base64"));
		}
	}

	//get preview canvas from dom
	private getCanvas(): CanvasRenderingContext2D {
		let canvas = document.getElementById("image-generator-canvas") as HTMLCanvasElement;
		if(canvas) {
			let ctx: CanvasRenderingContext2D | null = canvas.getContext("2d");
			if(ctx) {
				ctx.imageSmoothingEnabled = false;
				return ctx;
			}
		}
		throw Error("Could not find canvas to draw on");
	}

	//create new canvas with custom settings
	private newCanvas(): CanvasRenderingContext2D {
		let ctx = document.createElement("canvas").getContext("2d");
		if(ctx) {
			ctx.canvas.setAttribute("width", "64");
			ctx.canvas.setAttribute("height", "64");
			ctx.imageSmoothingEnabled = false;
			return ctx;
		}
		throw Error("Could not find canvas to draw on");
	}

	//calculate pixel changes between 2 frames of prepared images
	private async getPixelChanges(base: number, frame: number): Promise<number[][]> {
		let ctx: CanvasRenderingContext2D | null = this.newCanvas();
		if(ctx && this.state.canvasImages.length > 0) {
			this.drawFrame(base, ctx);
			let baseData = this.get2dPixelArray(ctx);
			this.drawFrame(frame, ctx);
			let frameData = this.get2dPixelArray(ctx);

			let changes: number[][] = [];
			for(var y = 0; y < 64; y++) {
				for(var x = 0; x < 64; x++) {
					if(baseData[y][x] !== frameData[y][x]) {
						changes.push([
							x,
							y,
							frameData[y][x]
						]);
					}
				}
			}

			return changes;
		} else {
			if(this.props.toast)
				this.props.toast.show({
					content: "No image to generate pixel art from",
					severity: 'error',
					closable: false
				});
			return [];
		}
	}

	//calculate flat bytearray for multidimensional animation array
	private getAnimationArray(changes: number[][][]): number[] {
		let byteArray: number[] = [];

		changes.forEach(frame => {
			//frame separator and for future metadata usage -> address [255,255] does not exist
			byteArray.push(0xFF, 0xFF, 0, 0);

			//pixelchanges
			frame.forEach(pixel => {
				byteArray.push(pixel[0], pixel[1], pixel[2] & 255, (pixel[2] >> 8) & 255);
			});
		});

		return byteArray;
	}

	//set canvas pixels by flat bytearray
	private setPixels(pixels: number[], canvas: CanvasRenderingContext2D = this.getCanvas()) {		
		var id = canvas.createImageData(1,1);

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				var location = (y * 64 + x) * 2;
				var pixel = rgb888((pixels[location + 1] << 8) + pixels[location]);
				id.data[0]   = pixel.r;
				id.data[1]   = pixel.g;
				id.data[2]   = pixel.b;
				id.data[3]   = 255;
				canvas.putImageData(id, x, y);
			}
		}
	}

	//remove current preview
	private clearCanvas() {		
		let canvas = this.getCanvas();
		var id = canvas.createImageData(1,1);
		id.data.fill(0);

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				canvas.putImageData(id, x, y);
			}
		}

		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
		this.setState({
			canvasImages: [],
			frameInterval: null,
			name: "",
			rendering: false,
			viewHistory: false
		});
	}

	//get flat bytearray from canvas
	private getPixelArray(canvas: CanvasRenderingContext2D = this.getCanvas()): number[] {
		let pixels = [];
		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				var pixel = canvas.getImageData(x, y, 1, 1);
				let pixelInt = pixel.data[3] === 0 ? 0 : rgb565(pixel.data[0], pixel.data[1], pixel.data[2]);
				pixels.push(pixelInt & 255, (pixelInt >> 8) & 255);
			}
		}
		return pixels;
	}

	//get 2d pixel array from canvas (pixel[y][x])
	private get2dPixelArray(canvas: CanvasRenderingContext2D = this.getCanvas()): number[][] {
		let pixels: number[][] = [];
		for(var y = 0; y < 64; y++) {
			pixels[y] = [];
			for(var x = 0; x < 64; x++) {
				var pixel = canvas.getImageData(x, y, 1, 1);
				pixels[y][x] = pixel.data[3] === 0 ? 0 : rgb565(pixel.data[0], pixel.data[1], pixel.data[2])
			}
		}
		return pixels;
	}

	//save preview image/animation to cache
	private cacheImage(name: string, image: number[], animation: number[][][] = [], preview: string = "") {
		let index: number = 0;
		let cacheItem: string | null = null;
		do {
			index++;
			cacheItem = localStorage.getItem("image-" + index);
			if(index > 40) {
				index = 1;
				break;
			}
		} while(cacheItem != null);

		let cachedImage: ICachedImage = {
			id: index,
			name: name,
			preview: preview,
			created: moment().unix(),
			animation: animation,
			image: image,
		};
		localStorage.setItem("image-" + index, JSON.stringify(cachedImage));
		this.setState({
			cachedItems: [...this.state.cachedItems, cachedImage]
		});
	}

	//load images from cache into state
	private loadCache() {
		let images: ICachedImage[] = [];
		for(let index: number = 1; index <= 40; index++) {
			let cacheItem = localStorage.getItem("image-" + index);
			if(cacheItem) {
				let cachedImage = JSON.parse(cacheItem) as ICachedImage;
				if(cachedImage.id && cachedImage.name && cachedImage.image) {
					images.push(cachedImage);
				}
			}
		}

		this.setState({
			cachedItems: images.sort((i1, i2) => i1.created - i2.created)
		});
	}

	//load image from cache into preview
	private loadFromCache(name: string, image: number[], animation: number[][][]) {
		let ctx = this.newCanvas();
		let frames: ImageData[] = [];

		//load image on canvas
		this.setPixels(image, ctx);
		
		//load frames
		var id = ctx.createImageData(1,1);
		animation.forEach(frame => {
			//push before foreach to first save base image without animation and dont push loopback frame (last frame)
			frames.push(ctx.getImageData(0, 0, 64, 64));
			frame.forEach(change => {
				var pixel = rgb888(change[2]);
				id.data[0]   = pixel.r;
				id.data[1]   = pixel.g;
				id.data[2]   = pixel.b;
				id.data[3]   = 255;
				ctx.putImageData(id, change[0], change[1]);
			});
		});

		this.currentFrame = 0;
		this.setState({
			name: name,
			canvasImages: frames,
			viewHistory: false,
		}, () => setTimeout(() => this.drawFromImages(), 50));
	}
}