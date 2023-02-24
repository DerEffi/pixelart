import DataService from '../../../services/DataService';
import React from 'react';

export interface IWifiSettingsComponentProps {
	dataService: DataService;
}

interface IWifiSettingsComponentState {

}

export default class WifiSettings extends React.Component<IWifiSettingsComponentProps, IWifiSettingsComponentState> {

	constructor(props: IWifiSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD Wifi settings

				</div>
				
            </div>
        );
    }
}