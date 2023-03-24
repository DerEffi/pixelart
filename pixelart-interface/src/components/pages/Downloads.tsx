import DataService from '../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { Button } from 'primereact/button';
import { Link } from 'react-router-dom';
import axios from 'axios';
import JSZip from 'jszip';
import { VersionDetails } from '../../models/Version';
import { Messages } from 'primereact/messages';
import { BiInfoCircle } from 'react-icons/bi';

export interface IDownloadsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IDownloadsComponentState {
    downloading: boolean;
}

export default class Downloads extends React.Component<IDownloadsComponentProps, IDownloadsComponentState> {

    private messages: Messages | null = null;

	constructor(props: IDownloadsComponentProps) {
		super(props);

		this.state = {
            downloading: false
		}
	}

    componentDidMount(): void {
        if(this.messages)
            this.messages.show({
                closable: false,
                sticky: true,
                severity: "info",
                content: <>
                    <BiInfoCircle className='message-icon'/>
                    <div>
                        <p>
                            Download the current version of the firmware, running the LED-Panel itself, and the webinterface for remote access from any device on your network.
                            After downloading you can place the files in the root directory of your sd card to start the update.
                        </p>
                        <p>
                            You can also automatically update your device under <Link to={"/settings/system"}>'Settings' &gt; 'System'</Link> if you have established a connection with your device.
                        </p>
                    </div>
                </>,
            });
    }

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content full-centered">
					
                    <table className='update-table'>
                        <tbody>
                            <tr>
                                <td>Firmware</td>
                                <td>{this.props.dataService.newestFirmware?.version || "-"}</td>
                                <td><Button disabled={!this.props.dataService.newestFirmware || this.state.downloading} onClick={() => { this.downloadFiles(this.props.dataService.newestFirmware); }} >Download</Button></td>
                            </tr>
                            <tr>
                                <td>Webinterface</td>
                                <td>{this.props.dataService.newestWebinterface?.version || "-"}</td>
                                <td><Button disabled={!this.props.dataService.newestWebinterface || this.state.downloading} onClick={() => { this.downloadFiles(this.props.dataService.newestWebinterface); }} >Download</Button></td>
                            </tr>
                        </tbody>
                    </table>

                    <Messages ref={(el) => this.messages = el} style={{textAlign: 'left'}} />
                </div>
				
            </div>
        );
    }

    public async downloadFiles(version?: VersionDetails) {
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

            //create zip
            let zip = new JSZip();
			zip.file("_place inside root folder on your sd card_", "");
            files.forEach(file => {
                zip.file(file.path, file.content);
            });

			zip.generateAsync({type: "blob"}).then((blob: Blob) => {
				let file = document.createElement("a");
				file.href = window.URL.createObjectURL(blob);
				file.download = `pixelart-${version.type}-${version.version}.zip`;
				file.click();
				file.remove();
			});

        } catch(e) {
            if(this.props.toast)
                this.props.toast.show({
                    content: "There was an issue with your download",
                    severity: 'error',
                    closable: false
                });
        }

        this.setState({
            downloading: false
        });
    }
}