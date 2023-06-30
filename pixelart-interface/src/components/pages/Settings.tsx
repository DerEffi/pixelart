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
import SystemSettings from './Settings/SystemSettings';
import SocialsSettings from './Settings/SocialsSettings';
import TimeSettings from './Settings/TimeSettings';
import DisplaySettings from './Settings/DisplaySettings';
import PictureSettings from './Settings/PictureSettings';
import { Toast } from 'primereact/toast';

export interface ISettingsComponentProps {
	dataService: DataService;
    toast: Toast | null;
    advanced: boolean;
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
            <>

                    {this.props.dataService.getStatus() === Status.connected &&
                        <Routes>
                            <Route path="/display" element={<DisplaySettings advanced={this.props.advanced} dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/pictures" element={<PictureSettings advanced={this.props.advanced} dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/time" element={<TimeSettings advanced={this.props.advanced} dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/wifi" element={<WifiSettings advanced={this.props.advanced} dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/socials" element={<SocialsSettings advanced={this.props.advanced} dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="/system" element={<SystemSettings advanced={this.props.advanced} dataService={this.props.dataService} toast={this.props.toast} />} />
                            <Route path="*" element={<NotFound/>} />
                        </Routes>
                    }

                    {this.props.dataService.getStatus() === Status.unauthorized &&
                        <div className='fullwidth full-centered'>
                            <div className="content">
                                <span className="font-xxl primary">
                                    <BiLockAlt/>
                                </span>
                                
                                <ProgressBar mode='indeterminate' className='progress-bar' />

                                <p>
                                    Please press the 'Menu' button on your device
                                    <br/>
                                    to establish a connection
                                </p>
                            </div>
                        </div>
                    }

                    {this.props.dataService.getStatus() === Status.disconnected &&
                        <div className='fullwidth full-centered'>
                            <div className="content">
                                <span className="font-xxl primary">
                                    <VscDebugDisconnect/>
                                </span>
                                
                                <p>
                                    Could not establish a connection with your device
                                    <br/>
                                    on the address '{this.props.dataService.getDeviceAddress()}'
                                </p>
                            </div>
                        </div>
                    }

                    {this.props.dataService.getStatus() === Status.pending &&
                        <div className='fullwidth full-centered'>
                            <div className="content">
                                <ProgressSpinner strokeWidth='3' />
                            </div>
                        </div>
                    }

            </>
        );
    }
}