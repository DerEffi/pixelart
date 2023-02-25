import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface ITimeSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface ITimeSettingsComponentState {

}

export default class TimeSettings extends React.Component<ITimeSettingsComponentProps, ITimeSettingsComponentState> {

	constructor(props: ITimeSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD Time settings

				</div>
				
            </div>
        );
    }
}