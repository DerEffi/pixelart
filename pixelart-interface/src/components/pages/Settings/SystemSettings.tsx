import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { Chart } from 'primereact/chart';
import moment from 'moment';
import 'moment-duration-format';
import { Button } from 'primereact/button';
import axios from 'axios';
import { VersionDetails } from '../../../models/Version';

export interface ISystemSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface ISystemSettingsComponentState {
	downloading: boolean;
}

const maxDataPoints: number = 50;

export default class SystemSettings extends React.Component<ISystemSettingsComponentProps, ISystemSettingsComponentState> {

	constructor(props: ISystemSettingsComponentProps) {
		super(props);

		this.state = {
			downloading: false
		};
	}

    public render() {
		let statisticsData = this.props.dataService.statistics.slice(this.props.dataService.statistics.length >= maxDataPoints ? this.props.dataService.statistics.length - maxDataPoints : 0, this.props.dataService.statistics.length);

        return(
            <div className='fullwidth'>

				<div className="content">

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
                                <td><Button disabled={!this.props.dataService.newestFirmware || !this.props.dataService.data.display?.version || this.state.downloading} onClick={() => { this.performUpdate(this.props.dataService.newestFirmware); }} >Update</Button></td>
                            </tr>
                            <tr>
                                <td>Webinterface</td>
                                <td>{this.props.dataService.deviceWebinterfaceVersion?.version || "-"}</td>
                                <td>{this.props.dataService.newestWebinterface?.version || "-"}</td>
                                <td><Button disabled={!this.props.dataService.newestWebinterface || !this.props.dataService.deviceWebinterfaceVersion || this.state.downloading} onClick={() => { this.performUpdate(this.props.dataService.newestWebinterface); }} >Update</Button></td>
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
                        console.dir(version);
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
			//TODO

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
}