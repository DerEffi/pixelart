import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { Chart } from 'primereact/chart';
import moment from 'moment';
import 'moment-duration-format';
import { Button } from 'primereact/button';
import axios from 'axios';
import { VersionDetails } from '../../../models/Version';
import * as semver from 'semver';
import { asyncTimeout } from '../../../services/Helper';
import { APIError } from '../../../models/Errors';
import { ConfirmDialog } from 'primereact/confirmdialog';
import { v4 as uuid } from 'uuid';
import { Messages } from 'primereact/messages';
import { BiInfoCircle } from 'react-icons/bi';

export interface ISystemSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface ISystemSettingsComponentState {
	downloading: boolean;
	showResetConfirmDialog: boolean;
}

const maxDataPoints: number = 50;

export default class SystemSettings extends React.Component<ISystemSettingsComponentProps, ISystemSettingsComponentState> {

    private messages: Messages | null = null;

	constructor(props: ISystemSettingsComponentProps) {
		super(props);

		this.state = {
			downloading: false,
			showResetConfirmDialog: false
		};
	}

    public render() {
		let statisticsData = this.props.dataService.statistics.slice(this.props.dataService.statistics.length >= maxDataPoints ? this.props.dataService.statistics.length - maxDataPoints : 0, this.props.dataService.statistics.length);

        return(
            <div className='fullwidth'>

				<div className="content">

					<ConfirmDialog
						visible={this.state.showResetConfirmDialog}
						draggable={false}
						dismissableMask
						blockScroll
						onHide={() => this.setState({showResetConfirmDialog: false})}
						rejectLabel="Abort"
						accept={() => this.reset()}
						acceptLabel='Reset'
						acceptIcon="pi pi-trash"
						message={
							<div style={{textAlign: "center"}}>
								<p>Resetting the device to "Factory" Settings
									<br/>will remove all your adjustments</p>
								<p>Do you want to proceed?</p>
							</div>
						}
					/>

					<Messages ref={(el) => this.messages = el} style={{textAlign: 'left'}} />

					<table className='update-table'>
						<thead>
							<tr>
								<td></td>
								<td>Current</td>
								<td>Newest</td>
							</tr>
						</thead>
                        <tbody>
                            <tr>
                                <td>Firmware</td>
                                <td>{this.props.dataService.data.display?.version || "-"}</td>
                                <td>{this.props.dataService.newestFirmware?.version || "-"}</td>
                                <td>
									<Button
										disabled={
											!this.props.dataService.newestFirmware
											|| this.state.downloading
											|| (
												!this.props.advanced 
												&& (
													!semver.valid(this.props.dataService.newestFirmware.version)
													|| !semver.valid(this.props.dataService.data.display?.version)
													|| !semver.gt(this.props.dataService.newestFirmware.version, this.props.dataService.data.display?.version || "0.0.1")
												)
											)
										}
										onClick={() => { this.performUpdate(this.props.dataService.newestFirmware); }}
										label='Update'
										iconPos='left'
										icon="pi pi-arrow-up"
									/>
								</td>
                            </tr>
                            <tr>
                                <td>Webinterface</td>
                                <td>{this.props.dataService.deviceWebinterfaceVersion?.version || "-"}</td>
                                <td>{this.props.dataService.newestWebinterface?.version || "-"}</td>
                                <td>
									<Button
										disabled={
											!this.props.dataService.newestWebinterface
											|| this.state.downloading
											|| (
												!this.props.advanced 
												&& (
													!semver.valid(this.props.dataService.newestWebinterface.version)
													|| !semver.valid(this.props.dataService.deviceWebinterfaceVersion?.version)
													|| !semver.gt(this.props.dataService.newestWebinterface.version, this.props.dataService.deviceWebinterfaceVersion?.version || "0.0.1")
												)
											)
										}
										onClick={() => { this.performUpdate(this.props.dataService.newestWebinterface); }}
										label='Update'
										iconPos='left'
										icon="pi pi-arrow-up"
									/>
								</td>
                            </tr>
							<tr>
								<td colSpan={4} style={{textAlign: "center"}}>
									<Button label='Logout on all devices' iconPos='left' icon="pi pi-sign-out" severity='warning' onClick={() => this.resetAPIKey()} />
								</td>
							</tr>
							{this.props.advanced &&
								<tr>
									<td colSpan={4} style={{textAlign: "center"}}>
										<Button label='Refresh Display' iconPos='left' icon="pi pi-desktop" severity='info' onClick={() => this.refresh()} />
									</td>
								</tr>
							}
							<tr>
								<td colSpan={4} style={{textAlign: "center"}}>
									<Button label='Restart' iconPos='left' icon="pi pi-refresh" severity='info' onClick={() => this.restart()} />
								</td>
							</tr>
							<tr>
								<td colSpan={4} style={{textAlign: "center"}}>
									<Button label='Reset' iconPos='left' icon="pi pi-trash" severity='danger' onClick={() => this.setState({showResetConfirmDialog: true})} />
								</td>
							</tr>
                        </tbody>
                    </table>
					
					{this.props.advanced &&
						<>
							<div>
								<div className='headline font-l'>Statistics</div>
							</div>

							<Chart
								className='chart'
								type="line"
								data={{
									datasets: [
										{
											label: "Internal Memory",
											borderColor: getComputedStyle(document.documentElement).getPropertyValue('--blue-500'),
											borderWidth: 2,
											pointHitRadius: 15,
											data: statisticsData.map(s => s.internal)
										},
										{
											label: "External Memory",
											borderColor: getComputedStyle(document.documentElement).getPropertyValue('--green-500'),
											borderWidth: 2,
											pointHitRadius: 15,
											data: statisticsData.map(s => s.external)
										}
									],
									labels: statisticsData.map(s => s.time.format("HH:mm:ss")),
								} as Chart.ChartData}
								options={{
									scales: {
										y: {
											type: "linear",
											position: "left"
										},
									},
									animation: false
								} as ChartOptions}
							/>

							<table className='table-group'>
								<tbody>
									<tr>
										<td>Display Refresh Rate</td>
										<td>{this.props.dataService.data.display?.refreshRate}Hz</td>
									</tr>
									<tr>
										<td>Image loaded from SD Card</td>
										<td>{this.props.dataService.data.images?.imageLoaded ? "Yes" : "No"}</td>
									</tr>
									<tr>
										<td>Soial Channels loaded</td>
										<td>{this.props.dataService.data.socials?.channelNumber}</td>
									</tr>
									<tr>
										<td>Online time</td>
										<td>{this.props.dataService.data.time ? moment.duration(this.props.dataService.data.time.online, "milliseconds").format("DD[d] HH[h] mm[m] ss[s]") : "-"}</td>
									</tr>
									<tr>
										<td>External RTC connected</td>
										<td>{this.props.dataService.data.time?.externalRTCConnected ? "Yes" : "No"}</td>
									</tr>
									<tr>
										<td>Free Memory</td>
										<td>{this.props.dataService.data.display?.freeMemory.toLocaleString()} byte</td>
									</tr>
									<tr>
										<td>Free Memory (SPI)</td>
										<td>{this.props.dataService.data.display?.freeSPIMemory.toLocaleString()} byte</td>
									</tr>
								</tbody>
							</table>

						</>
					}

				</div>
				
            </div>
        );
    }

	public async performUpdate(version?: VersionDetails) {
        this.setState({
            downloading: true
        });

        try {

            if(!version)
                throw new Error("No version information available");

            //download files from update server
            let files: {content: ArrayBuffer, path: string}[] = await Promise.all(
                version.files.map(async path => 
                    new Promise((resolve, reject) => {
                        axios({
                            url: `${process.env.REACT_APP_UPDATE_SERVER}/${version.type}/${path}`,
                            method: "GET",
                            responseType: "arraybuffer"
                        }).then(resp => {
                            if(resp.status !== 200)
                                reject();
                            
                            resolve({
                                path: path,
                                content: resp.data
                            });
                        }).catch((e) => {
                            reject(e);
                        })
                    })
                )
            );

            //upload update to device
			if(version.type !== "webinterface" && version.type !== "firmware")
				throw new Error("Unexpected update type");

			let upload: FormData = new FormData();
			files.forEach(file => {
				upload.append(file.path, new Blob([file.content], {type: "octet/stream"}), uuid());
			})
			await this.props.dataService.uploadFiles(version.type, upload).then(async () => {
				if(this.props.toast)
					this.props.toast.show({
						content: "Update uploaded to your device",
						severity: 'success',
						closable: false
					});
				
				if(this.messages && version.type === "firmware")
					this.messages.show({
						closable: true,
						sticky: true,
						severity: "info",
						content: <>
							<BiInfoCircle className='message-icon'/>
							<div>
								<p>
									The update has been uploaded to your device and will apply on the next start.
									<br/>
									<br/>
									<div className='link' onClick={() => this.restart()}>Restart now</div>
								</p>
							</div>
						</>,
					});
			}).catch((e) => {
				if(this.props.toast)
					this.props.toast.show({
						content: "There was a problem uploding the update to your device",
						severity: 'error',
						closable: false
					});
			});

        } catch(e) {
            if(this.props.toast)
                this.props.toast.show({
                    content: "There was an issue updating your device",
                    severity: 'error',
                    closable: false
                });
        }

        this.setState({
            downloading: false
        });
    }

	private restart() {
		this.props.dataService.requestDevice("POST", "/api/restart")
			.then(async () => {
				await asyncTimeout(10000);
				await this.props.dataService.refresh().catch((e: APIError) => {
					if(this.props.toast)
						this.props.toast.show({
							content: e.message,
							severity: 'error',
							closable: false
						});
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

	private refresh() {
		this.props.dataService.requestDevice("POST", "/api/refresh")
			.then(async () => {
				await asyncTimeout(100);
				await this.props.dataService.refresh().catch((e: APIError) => {
					if(this.props.toast)
						this.props.toast.show({
							content: e.message,
							severity: 'error',
							closable: false
						});
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

	private reset() {
		this.props.dataService.requestDevice("POST", "/api/reset")
			.then(async () => {
				await asyncTimeout(10000);
				await this.props.dataService.refresh().catch((e: APIError) => {
					if(this.props.toast)
						this.props.toast.show({
							content: e.message,
							severity: 'error',
							closable: false
						});
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

	private resetAPIKey() {
		this.props.dataService.requestDevice("DELETE", "/api/apiKey")
			.then(async () => {
				await asyncTimeout(100);
				await this.props.dataService.refresh().catch((e: APIError) => {
					if(this.props.toast)
						this.props.toast.show({
							content: e.message,
							severity: 'error',
							closable: false
						});
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