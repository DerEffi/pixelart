import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface IPictureSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IPictureSettingsComponentState {

}

export default class PictureSettings extends React.Component<IPictureSettingsComponentProps, IPictureSettingsComponentState> {

	constructor(props: IPictureSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD Picture settings

				</div>
				
            </div>
        );
    }
}