import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface ISocialsSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface ISocialsSettingsComponentState {

}

export default class SocialsSettings extends React.Component<ISocialsSettingsComponentProps, ISocialsSettingsComponentState> {

	constructor(props: ISocialsSettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD Socials settings

				</div>
				
            </div>
        );
    }
}