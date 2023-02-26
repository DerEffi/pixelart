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

export interface IGeneratorComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IGeneratorComponentState {
	canvasImage: string;
	name: string;
}

export default class Generator extends React.Component<IGeneratorComponentProps, IGeneratorComponentState> {

	constructor(props: IGeneratorComponentProps) {
		super(props);

		this.state = {
			canvasImage: "",
			name: ""
		}
	}

	componentDidUpdate(prevProps: Readonly<IGeneratorComponentProps>, prevState: Readonly<IGeneratorComponentState>, snapshot?: any): void {
		
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
				
					<div id="image-generator-canvas-wrapper">
						<img alt="Hidden container for generating canvas content" src={this.state.canvasImage} id="image-generator-image" width={64} height={64} />
						<canvas id="image-generator-canvas" width={64} height={64} />
					</div>

					<div className="image-generator-menu input-group" style={{marginTop: "1rem"}}>
						<Tooltip target=".image-generator-menu-button" position="bottom" />

						<div className="input-group-field p-inputgroup">
    						<span className="p-inputgroup-addon">xxx - </span>
							<InputText placeholder='image name' value={this.state.name} onChange={(e) => this.setState({name: e.target.value})} />
						</div>
						<Button type="button" icon="pi pi-download" data-pr-tooltip="Download Image" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.downloadImage()}/>
						<Button type="button" icon="pi pi-upload" data-pr-tooltip="Upload to Device" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-success ml-auto" onClick={() => this.uploadToDevice()} disabled={this.props.dataService.getStatus() !== Status.connected}/>
						{this.props.advanced &&
							<>
								<Button type="button" icon="pi pi-copy" data-pr-tooltip="Copy hex" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-help ml-auto" onClick={() => this.copyByteArray()}/>
								<Button type="button" icon="pi pi-code" data-pr-tooltip="Copy code" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-help ml-auto" onClick={() => this.copyCArray()}/>
								<Button type="button" icon="pi pi-trash" data-pr-tooltip="Clear Image" className="image-generator-menu-button p-button-outlined p-button-rounded p-button-error ml-auto" onClick={() => this.clearImage()}/>
							</>
						}
					</div>

					<div>
						<Tooltip target=".custom-choose-btn" content="Select" position="bottom" />
						<Tooltip target=".custom-upload-btn" content="Generate as Animation" position="bottom" />
						<Tooltip target=".custom-cancel-btn" content="Clear" position="bottom" />

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
											<Tooltip target={".custom-upload-item-" + md5name} content="Generate" position="bottom" />
											<Tooltip target={".custom-cancel-item-" + md5name} content="Remove" position="bottom" />

											<Button type="button" icon="pi pi-upload" className={"custom-upload-item-" + md5name + " p-button-outlined p-button-rounded p-button-success ml-auto"} onClick={() => this.uploadImage(file)}/>
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
							chooseOptions={{ icon: 'pi pi-fw pi-images', iconOnly: true, className: 'custom-choose-btn p-button-rounded p-button-info p-button-outlined', label: "Select" }}
							uploadOptions={{ icon: 'pi pi-fw pi-upload', iconOnly: true, className: 'custom-upload-btn p-button-success p-button-rounded p-button-outlined', label: "Generate as Animation" }}
							cancelOptions={{ icon: 'pi pi-fw pi-times', iconOnly: true, className: 'custom-cancel-btn p-button-danger p-button-rounded p-button-outlined', label: "Clear" }}
							customUpload
							uploadHandler={(e) => this.uploadAnimation(e.files)}
							contentClassName="image-generator-upload"
						/>
					</div>

				</div>
				
            </div>
        );
    }

	private async uploadImage(file: File) {
		let byteArray = new Uint8Array(await file.arrayBuffer());
		let base64image = Buffer.from(byteArray).toString("base64");
		this.setState({
			canvasImage: "data:" + file.type + ";base64, " + base64image,
			name: file.name.substring(0, file.name.lastIndexOf(".") !== -1 ? file.name.lastIndexOf(".") : file.name.length)
		}, () => this.drawFromImage());
	}

	private async uploadAnimation(files: File[]) {
		//TODO
	}

	private drawFromImage() {
		window.requestAnimationFrame(() => {
			let image = document.getElementById("image-generator-image") as HTMLImageElement;
			let ctx = this.getCanvas();
			ctx.drawImage(image, 0, 0, 64, 64);
		});
	}

	private downloadImage() {
		let zip = new JSZip();
		zip.file("000 - " + this.state.name + "/image.pxart", this.getPixelArray());
		zip.file("place inside images folder on your device", "");
		zip.generateAsync({type: "blob"}).then((blob: Blob) => {
			let file = document.createElement("a");
			file.href = window.URL.createObjectURL(blob);
			file.download = this.state.name;
			file.click();
		});
	}

	private uploadToDevice() {
		//TODO
	}

	private copyCArray() {
		navigator.clipboard.writeText(c2dArray(this.get2dPixelArray()));
	}

	private copyByteArray() {
		navigator.clipboard.writeText(Buffer.from(this.getPixelArray()).toString("base64"));
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

	private setPixels(pixels: number[]) {
		let ctx = this.getCanvas();
		
		var id = ctx.createImageData(1,1);

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				var pixel = rgb888(pixels[y * 64 + x]);
				id.data[0]   = pixel.r;
				id.data[1]   = pixel.g;
				id.data[2]   = pixel.b;
				id.data[3]   = 255;
				ctx.putImageData(id, x, y);
			}
		}
	}

	private clearImage() {
		let ctx = this.getCanvas();
		
		var id = ctx.createImageData(1,1);
		id.data.fill(0);

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				ctx.putImageData(id, x, y);
			}
		}
	}

	private getPixelArray() {
		
		let ctx = this.getCanvas();
		let pixels = [];

		for(var y = 0; y < 64; y++) {
			for(var x = 0; x < 64; x++) {
				var pixel = ctx.getImageData(x, y, 1, 1);
				let pixelInt = pixel.data[3] === 0 ? 0 : rgb565(pixel.data[0], pixel.data[1], pixel.data[2]);
				pixels.push(pixelInt & 255);
				pixels.push((pixelInt >> 8) & 255);
			}
		}

		return pixels;
	}

	private get2dPixelArray() {

		let ctx = this.getCanvas();
		let pixels: number[][] = [];

		for(var y = 0; y < 64; y++) {
			pixels[y] = [];
			for(var x = 0; x < 64; x++) {
				var pixel = ctx.getImageData(x, y, 1, 1);
				pixels[y][x] = pixel.data[3] === 0 ? 0 : rgb565(pixel.data[0], pixel.data[1], pixel.data[2])
			}
		}
	
		return pixels;
	}
}