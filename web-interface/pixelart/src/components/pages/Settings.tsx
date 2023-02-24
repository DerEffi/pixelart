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

export interface ISettingsComponentProps {
	dataService: DataService;
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
                            <Route path="/wifi" element={<WifiSettings dataService={this.props.dataService} />} />
                            <Route path="*" element={<NotFound/>} />
                        </Routes>
                    }

                    {this.props.dataService.getStatus() === Status.unauthorized &&
                        <>
                            <span className="font-xxl primary">
                                <BiLockAlt/>
                            </span>
                            
                            <ProgressBar mode='indeterminate' className='progress-bar' />
                            
                            <br/>
                            Please press the 'Menu' button on your device
                            <br/>
                            to establish a connection
                        </>
                    }

                    {this.props.dataService.getStatus() === Status.disconnected &&
                        <>
                            <span className="font-xxl primary">
                                <VscDebugDisconnect/>
                            </span>
                            <br/>
                            Could not establish connection with your device
                            <br/>
                            on the address '{this.props.dataService.getDeviceAddress()}'
                        </>
                    }

                    {this.props.dataService.getStatus() === Status.pending &&
                        <>
                            <ProgressSpinner strokeWidth='3' />
                        </>
                    }

				</div>
				
            </div>
        );
    }
}