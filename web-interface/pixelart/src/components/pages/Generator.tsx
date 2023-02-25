import DataService from '../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface IGeneratorComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface IGeneratorComponentState {

}

export default class Generator extends React.Component<IGeneratorComponentProps, IGeneratorComponentState> {

	constructor(props: IGeneratorComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    TBD Image Generator

				</div>
				
            </div>
        );
    }
}