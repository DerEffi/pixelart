import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface IUpdateSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface IUpdateSettingsComponentState {

}

export default class UpdateSettings extends React.Component<IUpdateSettingsComponentProps, IUpdateSettingsComponentState> {

	constructor(props: IUpdateSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD Update settings

				</div>
				
            </div>
        );
    }
}