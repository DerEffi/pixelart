import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface IWifiSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
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