import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { IImageData, IImageOperation } from '../../../models/Settings';
import { Dialog } from 'primereact/dialog';
import { Button } from 'primereact/button';
import { OrderList } from 'primereact/orderlist';
import { Tooltip } from 'primereact/tooltip';
import { CgMenu } from 'react-icons/cg';
import { FiEdit2 } from 'react-icons/fi';
import { BiImage, BiMoviePlay } from 'react-icons/bi';
import { APIError } from '../../../models/Errors';
import { InputText } from 'primereact/inputtext';
import { padLeft } from '../../../services/Helper';
import { v4 as uuid } from 'uuid';
import { HiArrowNarrowRight } from 'react-icons/hi';

export interface IPictureSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IPictureSettingsComponentState {
	images?: IImageData[];
	editItem?: IImageData;
}

export default class PictureSettings extends React.Component<IPictureSettingsComponentProps, IPictureSettingsComponentState> {

	constructor(props: IPictureSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">

					<Dialog
						header="Edit Image"
						dismissableMask
						blockScroll
						draggable={false}
						resizable={false}
						visible={this.state.editItem !== undefined}
						className="images-edit content"
						headerClassName="images-edit-header"
						onHide={() => this.setState({editItem: undefined})}
					>
						<>
							{this.state.editItem &&
								<div className='socials-edit-form'>
									<div>
										<div className='socials-edit-label'>Image Name</div>
										<InputText value={(this.state.editItem.rename || this.state.editItem.folder).substring(6)} onChange={(e) => { this.updateEditItem(e.target.value) }} />
									</div>
					
									<div className="socials-edit-submit">
										<Button onClick={() => this.setState({editItem: undefined})}>Save</Button>
									</div>
								</div>
							}
						</>
					</Dialog>

					<div>
						<div className='headline'>Display Image</div>
						<div  className='input-group'>
							<Button icon="pi pi-angle-left" style={{borderRadius: "100%"}} onClick={() => this.setImageData({displayImage: (this.props.dataService.data.images && this.props.dataService.data.images.displayImage > 0 ? this.props.dataService.data.images.displayImage - 1 : (this.props.dataService.data.images?.imageNumber || 1) - 1)})} />
							{this.props.dataService.data.images?.displayImage}
							<Button icon="pi pi-angle-right" style={{borderRadius: "100%"}} onClick={() => this.setImageData({displayImage: this.props.dataService.data.images ? this.props.dataService.data.images.displayImage + 1 : 1})} />
						</div>
					</div>

					{!this.props.advanced &&
						<>
							<OrderList
								value={this.state.images || this.props.dataService.data.images?.images}
								header="Images"
								dragdrop
								itemTemplate={(item: IImageData) =>
									<div className='images-list-item'>
										<Tooltip target={".images-list-item-tooltip-" + item.prefix} position='bottom' />
										<CgMenu className='images-list-item-icon'/>
										<FiEdit2 style={{cursor: "pointer"}} data-pr-tooltip="Edit Item" className={"images-list-item-input-icon images-list-item-tooltip-" + item.prefix} onClick={() => this.setState({editItem: item})} />
										
										<div  className='images-list-item-input'>
											{item.animated ? <BiMoviePlay data-pr-tooltip="Animation" className={"images-list-item-input-icon images-list-item-tooltip-" + item.prefix} /> : <BiImage data-pr-tooltip="Image" className={"images-list-item-input-icon images-list-item-tooltip-" + item.prefix} />}
											<div>{item.rename || item.folder}</div>
										</div>
									</div>
								}
								onChange={(e) => this.updateImages(e.value)}
							/>	
						</>
					}

					{this.props.advanced &&
						<table className='table-group'>
							<tbody>
								{(this.state.images || this.props.dataService.data.images?.images || []).map((image) => 
									<tr>
										<td>{image.animated ? <BiMoviePlay className="images-list-item-input-icon" /> : <BiImage data-pr-tooltip="Image" className="images-list-item-input-icon" />}</td>
										<td>{image.folder}</td>
										<td><HiArrowNarrowRight className='images-list-item-input-icon'/></td>
										<td><InputText className='input-group-field' value={image.rename || image.folder} onChange={(e) => this.advancedEditItem(image, e.target.value)} /></td>
									</tr>
								)}
							</tbody>
						</table>
					}

					<div>
						<div className='input-group'>
							<Button disabled={!this.state.images} onClick={() => this.sendFileOperations(this.state.images || [])} label='Apply' iconPos='left' icon="pi pi-send" />
						</div>
					</div>

				</div>

            </div>
        );
    }

	private updateEditItem(name: string) {
		if(this.state.editItem !== undefined) {
			let image = this.state.editItem;
			image.rename = padLeft(image.prefix, 3) + " - " + name;
			this.setState({
				editItem: image
			});
		}
	}

	private advancedEditItem(image: IImageData, name: string) {
		let images = this.state.images || this.props.dataService.data.images?.images || [];
		images.forEach(i => {
			if(i.folder === image.folder)
				i.rename = name;
		});
		this.setState({
			images: images
		});
	}

	private updateImages(images: IImageData[]) {
		let counter = 0;

		this.setState({
			images: images.map(i => {
				i.prefix = ++counter;
				i.rename = `${padLeft(counter, 3)} - ${(i.rename || i.folder).substring(6)}`
				return i;
			})
		});
	}

	private async setImageData(data: any) {
        await this.props.dataService.requestDevice("POST", "/api/images", data)
			.then(() => this.props.dataService.refreshImages())
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private async sendFileOperations(images: IImageData[]) {
		let sourceOperations: IImageOperation[] = [];
		let deleteOperations: IImageOperation[] = [];
		let targetOperations: IImageOperation[] = [];

		images.forEach(image => {
			if(image.rename) {
				let id: string = uuid();
				sourceOperations.push({src: image.folder, dst: id});
				deleteOperations.push({src: image.rename});
				targetOperations.push({src: id, dst: image.rename});
			}
		});

		await this.props.dataService.requestDevice("POST", "/api/images", {imageOperations: [...sourceOperations, ...deleteOperations, ...targetOperations]})
			.then(() => setTimeout(() => {
				this.props.dataService.refreshImages()
				.then(() => {
					this.setState({
						editItem: undefined,
						images: undefined
					});
				});
			}, 500))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
	}
}