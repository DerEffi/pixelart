import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { Button } from 'primereact/button';
import { APIError } from '../../../models/Errors';
import { InputText } from 'primereact/inputtext';
import { InputTextarea } from 'primereact/inputtextarea';
import axios from 'axios';
import { SocialItem } from '../../../models/Socials';
import { Dialog } from 'primereact/dialog';
import { OrderList } from 'primereact/orderlist';
import { IoShareSocial } from 'react-icons/io5';
import { BsInstagram, BsTwitch, BsYoutube } from 'react-icons/bs';
import { Link } from 'react-router-dom';
import {v4 as uuid} from 'uuid';
import { CgMenu } from 'react-icons/cg';
import { Dropdown } from 'primereact/dropdown';
import { FiEdit2 } from 'react-icons/fi';
import { Tooltip } from 'primereact/tooltip';

export interface ISocialsSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface ISocialsSettingsComponentState {
	request?: string;
	apiKey?: string;
	server?: string;
	testResponse: SocialItem[];
	loadingTest: boolean;
	socialList: SocialItem[];
	lastDataServiceRequest: string;
	editItem?: string;
}

export default class SocialsSettings extends React.Component<ISocialsSettingsComponentProps, ISocialsSettingsComponentState> {

	constructor(props: ISocialsSettingsComponentProps) {
		super(props);

		this.state = {
			lastDataServiceRequest: this.props.dataService.data.socials?.request || "",
			testResponse: [],
			loadingTest: false,
			socialList: this.decodeSocialRequest()
		};
	}

	componentDidUpdate(prevProps: Readonly<ISocialsSettingsComponentProps>, prevState: Readonly<ISocialsSettingsComponentState>, snapshot?: any): void {
		if(this.state.request === undefined && this.state.lastDataServiceRequest !== (this.props.dataService.data.socials?.request || ""))
			this.setState({
				lastDataServiceRequest: (this.props.dataService.data.socials?.request || ""),
				socialList: this.decodeSocialRequest((this.props.dataService.data.socials?.request || ""))
			});
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">

					<Dialog
						header="Edit Social"
						dismissableMask
						blockScroll
						draggable={false}
						resizable={false}
						visible={this.state.editItem !== undefined}
						className="socials-edit content"
						headerClassName="socials-edit-header"
						onHide={() => this.setState({editItem: undefined})}
					>
						<>
							{this.renderEditDialog(this.state.editItem as string)}
						</>
					</Dialog>

					<Dialog
						header="Socials"
						dismissableMask
						blockScroll
						draggable={false}
						resizable={false}
						visible={this.state.testResponse.length > 0}
						className="socials-test content"
						headerClassName="socials-test-header"
						onHide={() => this.setState({testResponse: []})}
					>
						<div className='socials-test-items'>
							{this.state.testResponse.map(social => {

								let icon: JSX.Element = <IoShareSocial/>;
								let url: string = "";
								let color: string = "#FFFFFF";
								switch(social.t) {
									case "t":
										icon = <BsTwitch/>;
										url = "https://www.twitch.tv/" + social.c;
										color = "#6441a5";
										break;
									case "y":
										icon = <BsYoutube/>;
										url = "https://www.youtube.com/" + social.c;
										color = "#FF0000";
										break;
									case "i":
										icon = <BsInstagram/>;
										url = "https://www.instagram.com/" + social.c;
										color = "#E1306C";
										break;
								}

								return(
									<Link key={social.t + social.c} target={"_blank"} to={url}>
										<div className='socials-test-item'>
											<div style={{color: color}} className='socials-test-item-icon'>{icon}</div>
											<div>{social.d || social.c}</div>
											<div>{social.f}</div>
											<div>{social.v}</div>
										</div>
									</Link>
								);
							})}
						</div>
					</Dialog>

					<div>
						<div className='headline'>Display Channel</div>
						<div  className='input-group'>
							<Button icon="pi pi-angle-left" style={{borderRadius: "100%"}} onClick={() => this.setSocialData({displayChannel: (this.props.dataService.data.socials && this.props.dataService.data.socials.displayChannel > 0 ? this.props.dataService.data.socials.displayChannel - 1 : (this.props.dataService.data.socials?.channelNumber || 1) - 1)})} />
							{this.props.dataService.data.socials?.displayChannel}
							<Button icon="pi pi-angle-right" style={{borderRadius: "100%"}} onClick={() => this.setSocialData({displayChannel: this.props.dataService.data.socials ? this.props.dataService.data.socials.displayChannel + 1 : 1})} />
						</div>
					</div>

					{!this.props.advanced &&
						<>
							<OrderList
								value={this.state.socialList}
								header="Socials"
								dragdrop
								itemTemplate={(item: SocialItem) =>
									<div className='socials-list-item'>
										<Tooltip target={".socials-list-item-tooltip-" + item.id} position='bottom' />
										<CgMenu className='socials-list-item-icon'/>
										<FiEdit2 style={{cursor: "pointer"}} data-pr-tooltip="Edit Item" className={"socials-list-item-input-icon socials-list-item-tooltip-" + item.id} onClick={() => this.setState({editItem: item.id})} />
										
										<div  className='socials-list-item-input'>
											{item.t === "t" &&
												<BsTwitch data-pr-tooltip="Twitch" className={"socials-list-item-input-icon socials-list-item-tooltip-" + item.id}/>
											}
											{item.t === "y" &&
												<BsYoutube data-pr-tooltip="Youtube" className={"socials-list-item-input-icon socials-list-item-tooltip-" + item.id}/>
											}
											{item.t === "i" &&
												<BsInstagram data-pr-tooltip="Instagram" className={"socials-list-item-input-icon socials-list-item-tooltip-" + item.id}/>
											}
											
											<div>{item.d || item.c}</div>
											<div>{item.d ? `(${item.c})` : ""}</div>
										</div>
									</div>
								}
								onChange={(e) => this.updateRequest(e.value)}
							/>	
						</>
					}
					
                    {this.props.advanced &&
						<>
							<div>
								<div className='headline'>Request</div>
								<div className='input-group'>
									<InputTextarea className='input-group-field' value={this.state.request || this.props.dataService.data.socials?.request} onChange={(e) => this.setState({request: e.target.value, socialList: this.decodeSocialRequest()})} />
									<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.request} onClick={() => this.setSocialRequest(this.state.request || "")} />
								</div>
							</div>

							<div>
								<div className='headline'>Server</div>
								<div className='input-group'>
									<InputText className='input-group-field' value={this.state.server || this.props.dataService.data.socials?.server} onChange={(e) => this.setState({server: e.target.value})} />
									<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.server} onClick={() => this.setSocialServer(this.state.server || "")} />
								</div>
							</div>

							<div>
								<div className='headline'>APIKey</div>
								<div className='input-group'>
									<InputText className='input-group-field' value={this.state.apiKey || this.props.dataService.data.socials?.apiKey} onChange={(e) => this.setState({apiKey: e.target.value})} />
									<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.apiKey} onClick={() => this.setSocialApiKey(this.state.apiKey || "")} />
								</div>
							</div>
						</>
					}

					<div>
						<div className='input-group'>
							<Button onClick={() => this.testSocials(this.state.server || this.props.dataService.data.socials?.server || "", this.state.apiKey || this.props.dataService.data.socials?.apiKey || "", this.state.request || this.props.dataService.data.socials?.request || "[]")} disabled={this.state.loadingTest} label='Test' iconPos='left' icon="pi pi-check-square" />
						</div>

						{!this.props.advanced &&
							<>
								<br/>
								<div className='input-group'>
									<Button disabled={!this.state.request} onClick={() => this.setSocialRequest(this.state.request || "")} label='Apply' iconPos='left' icon="pi pi-send" />
								</div>
							</>
						}
					</div>

				</div>

            </div>
        );
    }

	private renderEditDialog(id: string): JSX.Element {
		let items: SocialItem[] = this.state.socialList.filter(i => i.id === id);
		if(items.length !== 1)
			return <></>;

		let item = items[0];
		return(
			<div className='socials-edit-form'>
				<div>
					<div className='socials-edit-label'>Platform</div>
					<Dropdown
						value={item.t}
						placeholder='Platform'
						onChange={(e) => { let newItem = item; newItem.t = e.value; this.updateSocialItem(newItem); }}
						options={[
							{value: "t", label: "Twitch"},
							{value: "y", label: "Youtube"},
							{value: "i", label: "Instagram"}
						]}
					/>
				</div>
				<div>
					<div className='socials-edit-label'>Display Name</div>
					<InputText value={item.d} placeholder={item.c} onChange={(e) => { item.d = e.target.value; this.updateSocialItem(item); }} />
				</div>
				<div>
					<div className='socials-edit-label'>Channel Name</div>
					<InputText value={item.c} placeholder="channel name" onChange={(e) => { item.c = e.target.value; this.updateSocialItem(item); }} />
				</div>

				<div className="socials-edit-submit">
					<Button>Save</Button>
				</div>
			</div>
		);
	}

	private decodeSocialRequest(request: string = (this.state ? this.state.request : undefined) || this.props.dataService.data.socials?.request || "[]"): SocialItem[] {
		try {
			return (JSON.parse(request) as SocialItem[]).map((i) => { i.id = uuid(); return i;});
		} catch {
			return [];
		}
	}

	private updateRequest(items: SocialItem[]) {	
		this.setState({
			socialList: items,
			request: JSON.stringify([...items].map(x => ({...x})).map(x => {x.id = undefined; return x;}))
		});
	}

	private updateSocialItem(newItem: SocialItem) {
		let updated = this.state.socialList.map(item => {
			if(item.id === newItem.id)
				item = newItem;
			return item;
		});
		this.setState({
			socialList: updated,
			request: JSON.stringify([...updated].map(x => ({...x})).map(x => {x.id = undefined; return x;}))
		});
	}

	private setSocialData(data: any) {
        this.props.dataService.requestDevice("POST", "/api/socials", data)
			.then(() => this.props.dataService.refreshSocials())
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private setSocialServer(server: string) {
        this.props.dataService.requestDevice("POST", "/api/socials", {server: server})
			.then(() => this.props.dataService.refreshSocials().then(() => this.setState({server: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private setSocialApiKey(apiKey: string) {
        this.props.dataService.requestDevice("POST", "/api/socials", {apiKey: apiKey})
			.then(() => this.props.dataService.refreshSocials().then(() => this.setState({apiKey: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private setSocialRequest(request: string) {
		//check for json format and strip unnecessary spaces
		try {
			let jsonRequest = JSON.parse(request);
			request = JSON.stringify(jsonRequest);
		} catch {
			if(this.props.toast)
				this.props.toast.show({
					content: "Your request data is not in the exspected format",
					severity: 'error',
					closable: false
				});
			return;
		}

        this.props.dataService.requestDevice("POST", "/api/socials", {request: request})
			.then(() => this.props.dataService.refreshSocials().then(() => this.setState({request: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private testSocials(server: string, apiKey: string, request: string) {

		this.setState({loadingTest: true});

		let jsonRequest;
		try {
			jsonRequest = JSON.parse(request);
		} catch {
			if(this.props.toast)
				this.props.toast.show({
					content: "Your request data is not in the exspected format",
					severity: 'error',
					closable: false
				});
			return;
		}

		axios.post(server + "/socials", jsonRequest, {
			headers: {
				"Accept": "application/json",
				"Content-Type": "application/json",
				"Authorization": apiKey
			}
		})
		.then((resp) => {
			if(resp.status !== 200)
				throw new Error("" + resp.status);
			
			this.setState({testResponse: resp.data as SocialItem[], loadingTest: false});
		})
		.catch(() => {
			if(this.props.toast)
				this.props.toast.show({
					content: "The server reporeted an error with your request",
					severity: 'error',
					closable: false
				});
		});
	}
}