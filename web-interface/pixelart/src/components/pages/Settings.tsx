import DataService from '../../services/DataService';
import React from 'react';
import { Route, Routes } from 'react-router-dom';
import NotFound from './NotFound';
import WifiSettings from './Settings/WifiSettings';
import { Status } from '../../models/Status';
import { VscDebugDisconnect } from 'react-icons/vsc';
import { ProgressSpinner } from 'primereact/progressspinner';
import { BiLockAlt } from 'react-icons/bi';
import { ProgressBar } from 'primereact/progressbar';
import UpdateSettings from './Settings/UpdateSettings';
import SocialsSettings from './Settings/SocialsSettings';
import TimeSettings from './Settings/TimeSettings';
import DisplaySettings from './Settings/DisplaySettings';
import PictureSettings from './Settings/PictureSettings';
import { Toast } from 'primereact/toast';

export interface ISettingsComponentProps {
	dataService: DataService;
    toast: Toast | null;
}

interface ISettingsComponentState {

}

export default class Settings extends React.Component<ISettingsComponentProps, ISettingsComponentState> {

	constructor(props: ISettingsComponentProps) {
		super(props);

		this.state = {
            
		}
	}

    public render() {
        return(
            <div className={'fullwidth' + (this.props.dataService.getStatus() !== Status.connected ? " full-centered" : "")}>

				<div className="content">

                    {this.props.dataService.getStatus() === Status.connected &&
                        <Routes>
                            <Route path="/display" element={<DisplaySettings dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/pictures" element={<PictureSettings dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/time" element={<TimeSettings dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/wifi" element={<WifiSettings dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/socials" element={<SocialsSettings dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/updates" element={<UpdateSettings dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="*" element={<NotFound/>} />
                        </Routes>
                    }

                    {this.props.dataService.getStatus() === Status.unauthorized &&
                        <div>
                            <span className="font-xxl primary">
                                <BiLockAlt/>
                            </span>
                            
                            <ProgressBar mode='indeterminate' className='progress-bar' />
                            
                            <br/>
                            Please press the 'Menu' button on your device
                            <br/>
                            to establish a connection
                        </div>
                    }

                    {this.props.dataService.getStatus() === Status.disconnected &&
                        <div>
                            <span className="font-xxl primary">
                                <VscDebugDisconnect/>
                            </span>
                            <br/>
                            Could not establish connection with your device
                            <br/>
                            on the address '{this.props.dataService.getDeviceAddress()}'
                        </div>
                    }

                    {this.props.dataService.getStatus() === Status.pending &&
                        <div>
                            <ProgressSpinner strokeWidth='3' />
                        </div>
                    }

				</div>
				
            </div>
        );
    }
}