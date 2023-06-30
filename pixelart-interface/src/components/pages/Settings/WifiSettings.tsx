import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { IoWarningOutline } from 'react-icons/io5';
import { Messages } from 'primereact/messages';
import { Link } from 'react-router-dom';
import { Button } from 'primereact/button';
import { InputText } from 'primereact/inputtext';
import { SelectButton } from 'primereact/selectbutton';
import { SelectItemOptionsType } from 'primereact/selectitem';
import { BiHide, BiShow, BiWifi, BiWifi0, BiWifi1, BiWifi2, BiWifiOff } from 'react-icons/bi';
import { FaLock, FaLockOpen } from 'react-icons/fa';
import { ListBox } from 'primereact/listbox';
import { asyncTimeout, validateSSID, validateWPA } from '../../../services/Helper';
import { IWifiNetwork } from '../../../models/Settings';
import { APIError } from '../../../models/Errors';

export interface IWifiSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IWifiSettingsComponentState {
	ap?: boolean;
	apSSID?: string;
	apPassword?: string;
	connect?: boolean;
	ssid?: string;
	password?: string;
	showApPassword: boolean;
	showPassword: boolean;
	scanning: boolean;
	applying: boolean;
}

const switchButtonOptions: SelectItemOptionsType = [{label: "Off", value: false}, {label: "On", value: true}];

export default class WifiSettings extends React.Component<IWifiSettingsComponentProps, IWifiSettingsComponentState> {

    private messages: Messages | null = null;

	constructor(props: IWifiSettingsComponentProps) {
		super(props);

		this.state = {
			showApPassword: false,
			showPassword: false,
			scanning: false,
			applying: false
		};
	}

	componentDidMount(): void {
        if(!this.props.dataService.dismissedWifiWarning && this.messages)
            this.messages.show({
                closable: true,
                sticky: true,
                severity: "warn",
                content: <>
                    <IoWarningOutline className='message-icon'/>
                    <div>
						<p>
							Your device may disconnect temporarily on applying settings or searching for wifi networks. Depending on the settings you choose and the way you are connected to your device, you might loose connection permanently.
							<br/>
							Reenableing WiFi connection on your device is described under <Link to="/#wifi">'WiFi' on the 'Home' page</Link>.
						</p>
                    </div>
                </>,
            });
    }

    public render() {

		const validSSID: boolean = !this.state.ssid || validateSSID(this.state.ssid);
		const validApSSID: boolean = !this.state.apSSID || validateSSID(this.state.apSSID);
		const validPassword: boolean = !this.state.password || validateWPA(this.state.password);
		const validApPassword: boolean = !this.state.apPassword || validateWPA(this.state.apPassword);

        return(
            <div className='fullwidth'>

				<div className="content">
					
					<Messages ref={(el) => this.messages = el} style={{textAlign: 'left'}} onRemove={() => this.props.dataService.dismissWifiWarning()} />

					<div>
						<div className='headline'>Connect to WiFi Network</div>
						<div className='input-group'>
							<SelectButton value={this.state.connect !== undefined ? this.state.connect : this.props.dataService.data.wifi?.connect} onChange={(e) => {this.setState({connect: e.value})}} options={switchButtonOptions}/>
						</div>
					</div>

					{(this.state.connect !== undefined ? this.state.connect : this.props.dataService.data.wifi?.connect) &&
						<>

							<div>
								<div className='headline'>WiFi Network</div>
								{!this.props.advanced &&
									<>
										<div className='input-group'>
											<ListBox
												options={this.props.dataService.data.wifiScan}
												value={this.state.ssid || this.props.dataService.data.wifi?.ssid}
												optionValue="ssid"
												itemTemplate={(item: IWifiNetwork) =>
													<div className='wifi-network-item'>
														<div className='wifi-network-item-icon'>{item.rssi > -40 ? <BiWifi/> : item.rssi > -55 ? <BiWifi2/> : item.rssi > -70 ? <BiWifi1/> : item.rssi > -80 ? <BiWifi0/> : <BiWifiOff/>}</div>
														<div className='wifi-network-item-icon'>{item.encryption > 0 ? <FaLock style={{color: item.encryption > 2 ? "var(--text-color)" : "var(--red-700)"}}/> : <FaLockOpen/>}</div>
														<div className='wifi-network-item-label'>{item.ssid}</div>
													</div>
												}
												emptyMessage="No networks found"
												onChange={(e) => this.setState({ssid: e.target.value})}
											/>
										</div>
										<br/>
										<div className='input-group'>
											<Button onClick={() => this.scanWifi()} disabled={this.state.scanning} label='Scan Networks' iconPos='left' icon="pi pi-wifi" />
										</div>
									</>
								}
								{this.props.advanced &&
									<div className='input-group'>
										<InputText placeholder='(unchanged)' className={'input-group-field' + (validSSID ? "" : " p-invalid")} value={this.state.ssid === undefined ? this.props.dataService.data.wifi?.ssid : this.state.ssid} onChange={(e) => this.setState({ssid: e.target.value})} />
									</div>
								}
							</div>

							<div>
								<div className='headline'>WiFi Password</div>
								<div className='input-group'>
									<InputText placeholder='(unchanged)' type={this.state.showPassword ? "text" : "password"} className={'input-group-field' + (validPassword ? "" : " p-invalid")} value={this.state.password || ""} onChange={(e) => this.setState({password: e.target.value})} />
									{this.state.showPassword
										? <BiHide style={{cursor: "pointer"}} onClick={() => {this.setState({showPassword: false})}} />
										: <BiShow style={{cursor: "pointer"}} onClick={() => {this.setState({showPassword: true})}} />
									}
								</div>
							</div>
						</>
					}



					<div>
						<div className='headline'>Host WiFi Network</div>
						<div className='input-group'>
							<SelectButton value={this.state.ap !== undefined ? this.state.ap : this.props.dataService.data.wifi?.ap} onChange={(e) => {this.setState({ap: e.value})}} options={switchButtonOptions}/>
						</div>
					</div>

					{(this.state.ap !== undefined ? this.state.ap : this.props.dataService.data.wifi?.ap) &&
						<>
							<div>
								<div className='headline'>Hosted WiFi Name</div>
								<div className='input-group'>
									<InputText placeholder='(unchanged)' className={'input-group-field' + (validApSSID ? "" : " p-invalid")} value={this.state.apSSID === undefined ? this.props.dataService.data.wifi?.apSSID : this.state.apSSID} onChange={(e) => this.setState({apSSID: e.target.value})} />
								</div>
							</div>

							<div>
								<div className='headline'>Hosted WiFi Password</div>
								<div className='input-group'>
									<InputText placeholder='(unchanged)' type={this.state.showApPassword ? "text" : "password"} className={'input-group-field' + (validApPassword ? "" : " p-invalid")} value={this.state.apPassword === undefined ? this.props.dataService.data.wifi?.apPassword : this.state.apPassword} onChange={(e) => this.setState({apPassword: e.target.value})} />
									{this.state.showApPassword
										? <BiHide style={{cursor: "pointer"}} onClick={() => {this.setState({showApPassword: false})}} />
										: <BiShow style={{cursor: "pointer"}} onClick={() => {this.setState({showApPassword: true})}} />
									}
								</div>
							</div>

						</>
					}

					{this.props.advanced &&
						<table className='table-group'>
						<tbody>
							<tr>
								<td>Hosted Network IP</td>
								<td>{this.props.dataService.data.wifi?.apIP}</td>
							</tr>
							<tr>
								<td>Hostname</td>
								<td>{this.props.dataService.data.wifi?.hostname}</td>
							</tr>
							<tr>
								<td>MAC-Address</td>
								<td>{this.props.dataService.data.wifi?.mac}</td>
							</tr>
							<tr>
								<td>Connection Status</td>
								<td>{this.props.dataService.data.wifi?.setupComplete ? "Connected" : "Disconnected"}</td>
							</tr>
							<tr>
								<td>Connected Network IP</td>
								<td>{this.props.dataService.data.wifi?.ip}</td>
							</tr>
							<tr>
								<td>Connected Network RSSI</td>
								<td>{this.props.dataService.data.wifi?.rssi}</td>
							</tr>
						</tbody>
					</table>
					}

					<div>
						<div className='input-group'>
							<Button
								onClick={() => this.applyWifiSettings()}
								disabled={
									this.state.applying
									|| (
										this.state.ap === undefined
										&& !this.state.apPassword
										&& !this.state.apSSID
										&& this.state.connect === undefined
										&& !this.state.password
										&& !this.state.ssid
									)
									|| !validSSID
									|| !validPassword
									|| !validApSSID
									|| !validApPassword
								}
								label='Apply'
								iconPos='left'
								icon="pi pi-send"
							/>
						</div>
					</div>

				</div>
				
            </div>
        );
    }

	private async scanWifi() {
		this.setState({
			scanning: true
		});

		//first trigger scan, second time collect results
		await this.props.dataService.scanWifi();
		asyncTimeout(10000);
		await this.props.dataService.scanWifi();

		this.setState({
			scanning: false
		});
	}

	private applyWifiSettings() {
		this.setState({
			applying: true
		});

		let wifiSettings: any = {
			ap: this.state.ap,
			apSSID: this.state.apSSID,
			apPassword: this.state.apPassword,
			connect: this.state.connect,
			ssid: this.state.ssid,
			password: this.state.password,
		};

		//delete undefined values
		for(let setting in wifiSettings) {
			if(wifiSettings[setting] === undefined || wifiSettings[setting] === "")
				delete wifiSettings[setting];
		}
		
		this.props.dataService.requestDevice("POST", "/api/wifi", wifiSettings)
			.then(async () => {
				await asyncTimeout(10000);
				await this.props.dataService.refreshWifi().catch((e: APIError) => {
					if(this.props.toast)
						this.props.toast.show({
							content: e.message,
							severity: 'error',
							closable: false
						});
				});
				this.setState({
					ap: undefined,
					apSSID: undefined,
					apPassword: undefined,
					connect: undefined,
					ssid: undefined,
					password: undefined,
					applying: false
				});
			})
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