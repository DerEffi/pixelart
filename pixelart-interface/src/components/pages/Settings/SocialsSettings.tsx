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
}

export default class SocialsSettings extends React.Component<ISocialsSettingsComponentProps, ISocialsSettingsComponentState> {

	constructor(props: ISocialsSettingsComponentProps) {
		super(props);

		this.state = {
			testResponse: [],
			loadingTest: false
		};
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">

					<Dialog header="Socials" closable closeOnEscape dismissableMask draggable={false} resizable={false} maximizable={false} visible={this.state.testResponse.length > 0} className="socials-test content" headerClassName="socials-test-header" onHide={() => this.setState({testResponse: []})}>
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
							<Button icon="pi pi-angle-left" style={{borderRadius: "100%"}} onClick={() => this.setSocialData({displayChannel: (this.props.dataService.data.socials && this.props.dataService.data.socials.displayChannel > 0 ? this.props.dataService.data.socials.displayChannel - 1 : 999)})} />
							{this.props.dataService.data.socials?.displayChannel}
							<Button icon="pi pi-angle-right" style={{borderRadius: "100%"}} onClick={() => this.setSocialData({displayChannel: this.props.dataService.data.socials ? this.props.dataService.data.socials.displayChannel + 1 : 1})} />
						</div>
					</div>

					{!this.props.advanced &&
						<>
							<OrderList
								value={this.decodeSocialRequest(this.state.request || this.props.dataService.data.socials?.request || "[]")}
								header="Socials"
								dragdrop
								itemTemplate={(item: SocialItem) =>
									<>
										<div>{item.t}</div>
										<div>{item.c}</div>
										<div>{item.d}</div>
									</>
								}
								onChange={(e) => this.setState({request: JSON.stringify(e.value)})}
							/>	
						</>
					}
					
                    {this.props.advanced &&
						<>
							<div>
								<div className='headline'>Request</div>
								<div className='input-group'>
									<InputTextarea className='input-group-field' value={this.state.request || this.props.dataService.data.socials?.request} onChange={(e) => this.setState({request: e.target.value})} />
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
							<Button onClick={() => this.testSocials(this.state.server || this.props.dataService.data.socials?.server || "", this.state.apiKey || this.props.dataService.data.socials?.apiKey || "", this.state.request || this.props.dataService.data.socials?.request || "[]")} disabled={this.state.loadingTest} >Test Socials</Button>
						</div>

						{!this.props.advanced &&
							<>
								<br/>
								<div className='input-group'>
									<Button disabled={!this.state.request} onClick={() => this.setSocialRequest(this.state.request || "")}>Update Socials</Button>
								</div>
							</>
						}
					</div>

				</div>

            </div>
        );
    }

	public decodeSocialRequest(request: string): any[] {
		try {
			let requestItems = JSON.parse(request) as SocialItem[]
			return requestItems.map((i) => { i.id = uuid(); return i;});
		} catch {
			return [];
		}
	}

	public setSocialData(data: any) {
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

	public setSocialServer(server: string) {
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

	public setSocialApiKey(apiKey: string) {
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

	public setSocialRequest(request: string) {
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

	public testSocials(server: string, apiKey: string, request: string) {

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