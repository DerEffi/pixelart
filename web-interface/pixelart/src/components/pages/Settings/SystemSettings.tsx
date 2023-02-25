import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface ISystemSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface ISystemSettingsComponentState {

}

export default class SystemSettings extends React.Component<ISystemSettingsComponentProps, ISystemSettingsComponentState> {

	constructor(props: ISystemSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD System settings

				</div>
				
            </div>
        );
    }
}