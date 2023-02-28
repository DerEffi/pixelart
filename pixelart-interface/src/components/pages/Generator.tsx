import DataService from '../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { FileUpload } from 'primereact/fileupload';
import {Buffer} from 'buffer';
import { Tooltip } from 'primereact/tooltip';
import { Button } from 'primereact/button';
import {Md5} from 'ts-md5';
import { c2dArray, rgb565, rgb888 } from '../../services/Helper';
import { InputText } from 'primereact/inputtext';
import { Status } from '../../models/Status';
import JSZip from 'jszip';
import { GifReader } from 'omggif';
import { Knob } from 'primereact/knob';

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
}

export default class Generator extends React.Component<IGeneratorComponentProps, IGeneratorComponentState> {

	private currentFrame: number = 0;

	constructor(props: IGeneratorComponentProps) {
		super(props);

		this.state = {
			canvasImages: [],
			name: "",
			frameTime: 100,
			frameInterval: null
		}
	}

	componentWillUnmount(): void {
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
				
					<div id="image-generator-canvas-wrapper">
						<canvas id="image-generator-canvas" width={64} height={64} />
					</div>

					<div className="image-generator-menu input-group" style={{marginTop: "1rem"}}>
						<Tooltip target=".image-generator-menu-button" position="bottom" />

						<div className="input-group-field p-inputgroup">
    						<span className="p-inputgroup-addon">xxx - </span>
							<InputText placeholder='image name' value={this.state.name} onChange={(e) => this.setState({name: e.target.value})} />
						</div>
						<Button type="button" icon="pi pi-download" data-pr-tooltip="Download Pixelart" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.downloadImage()}/>
						<Button type="button" icon="pi pi-upload" data-pr-tooltip="Upload to Device" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.uploadToDevice()} disabled={this.props.dataService.getStatus() !== Status.connected}/>
					</div>

					{this.props.advanced &&
						<div className="input-group" style={{marginTop: "1rem"}}>
							<Tooltip target=".image-generator-menu-button-advanced" position="bottom" />

							<Button type="button" icon={"pi pi-" + (this.state.frameInterval ? "pause" : "play")} data-pr-tooltip="Play/Pause" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.playAnimation()}/>
							<Button type="button" icon="pi pi-step-forward" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-success ml-auto" data-pr-tooltip="Next Frame" disabled={this.state.frameInterval != null} onClick={() => this.drawFrame()} />
							<Knob className="image-generator-menu-button-advanced" data-pr-tooltip="Frametime (ms)" size={65} value={this.state.frameTime} step={25} min={25} max={500} onChange={(e) => this.adjustSpeed(e.value)} />
							<Button type="button" icon="pi pi-copy" data-pr-tooltip="Copy hex" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-help ml-auto" onClick={() => this.copyByteArray()}/>
							<Button type="button" icon="pi pi-code" data-pr-tooltip="Copy source code" className="image-generator-menu-button-advanced p-button-outlined p-button-rounded p-button-help ml-auto" onClick={() => this.copyCArray()}/>
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
									<div className={className} style={{ backgroundColor: 'transparent', display: 'flex', alignItems: 'center' }}>
										{chooseButton}
										{uploadButton}
										{cancelButton}
									</div>
								);
							}}
							itemTemplate={(file: any, props) => {
								let md5name = Md5.hashStr(file.name);
								return(
									<div className="image-generator-item">
										<img className="image-generator-item-image" alt={file.name} role="presentation" src={file.objectURL} width={100} height={100} />
										<div className="image-generator-item-buttons">
											<Tooltip target={".custom-upload-item-" + md5name} content="Draw Image" position="bottom" />
											<Tooltip target={".custom-cancel-item-" + md5name} content="Remove" position="bottom" />

											<Button type="button" icon="pi pi-pencil" className={"custom-upload-item-" + md5name + " p-button-outlined p-button-rounded p-button-success ml-auto"} onClick={() => this.uploadImages([file])}/>
											<Button type="button" icon="pi pi-times" className={"custom-cancel-item-" + md5name + " p-button-outlined p-button-rounded p-button-danger ml-auto"} onClick={props.onRemove}/>
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

	private drawFromImages() {
			this.currentFrame = 0;
			this.drawFrame();
			if(this.state.frameInterval)
				clearInterval(this.state.frameInterval);
			this.setState({frameInterval: setInterval(() => this.drawFrame(), this.state.frameTime)});
	}

	private drawFrame(frame: number = ++this.currentFrame, canvas: CanvasRenderingContext2D = this.getCanvas()) {
		if(this.state.canvasImages.length > 0) {
			frame = frame % this.state.canvasImages.length;
			let frameData = this.state.canvasImages[frame];
			if(frameData instanceof HTMLImageElement)
				canvas.drawImage(frameData, 0, 0, 64, 64);
			else if(frameData instanceof ImageData)
				canvas.putImageData(frameData, 0, 0);
			this.currentFrame = frame;
		} else {
			if(this.props.toast)
				this.props.toast.show({
					content: "No image to generate pixel art from",
					severity: 'error',
					closable: false
				});
		}
	}

	private playAnimation() {
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
		this.setState({
			frameInterval: this.state.frameInterval || this.state.canvasImages.length <= 1 ? null : setInterval(() => this.drawFrame(), this.state.frameTime)
		});
	}

	private adjustSpeed(time: number) {
		if(this.state.frameInterval)
			clearInterval(this.state.frameInterval);
		this.setState({
			frameTime: time,
			frameInterval: this.state.frameInterval ? setInterval(() => this.drawFrame(), time) : null
		});
	}

	private downloadImage() {
		let ctx: CanvasRenderingContext2D | null = document.createElement("canvas").getContext("2d");
		if(ctx && this.state.canvasImages.length > 0) {
			this.drawFrame(0, ctx);
			let animation: number[][][] = [];

			for(let i = 1; i < this.state.canvasImages.length; i++) {
				animation.push(this.getPixelChanges(i - 1, i));
			}

			let zip = new JSZip();
			zip.file("000 - " + this.state.name + "/image.pxart", this.getPixelArray(ctx));
			zip.file("place inside images folder on your sd card", "");
			if(animation.length > 0)
				zip.file("000 - " + this.state.name + "/animation.pxart", this.getAnimationArray(animation));
			zip.generateAsync({type: "blob"}).then((blob: Blob) => {
				let file = document.createElement("a");
				file.href = window.URL.createObjectURL(blob);
				file.download = this.state.name;
				file.click();
				file.remove();
			});

			ctx.canvas.remove();
		} else {
			if(this.props.toast)
				this.props.toast.show({
					content: "No image to generate pixel art from",
					severity: 'error',
					closable: false
				});
		}
	}

	private uploadToDevice() {
		//TODO
	}

	private copyCArray() {
		if(this.state.frameInterval) {
			//TODO
		} else {
			navigator.clipboard.writeText(c2dArray(this.get2dPixelArray()));
		}
	}

	private copyByteArray() {
		if(this.state.frameInterval) {
			//TODO
		} else {
			navigator.clipboard.writeText(Buffer.from(this.getPixelArray()).toString("base64"));
		}
	}

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

	private getPixelChanges(base: number, frame: number): number[][] {
		let ctx: CanvasRenderingContext2D | null = document.createElement("canvas").getContext("2d");
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

	private setPixels(pixels: number[], canvas: CanvasRenderingContext2D = this.getCanvas()) {		
		var id = canvas.createImageData(1,1);

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				var pixel = rgb888(pixels[y * 64 + x]);
				id.data[0]   = pixel.r;
				id.data[1]   = pixel.g;
				id.data[2]   = pixel.b;
				id.data[3]   = 255;
				canvas.putImageData(id, x, y);
			}
		}
	}

	private clearCanvas(canvas: CanvasRenderingContext2D = this.getCanvas()) {		
		var id = canvas.createImageData(1,1);
		id.data.fill(0);

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				canvas.putImageData(id, x, y);
			}
		}
	}

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
}