import DataService from '../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';

export interface IHomeComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface IHomeComponentState {

}

export default class Home extends React.Component<IHomeComponentProps, IHomeComponentState> {

	constructor(props: IHomeComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    {/* TDB */}

				</div>
				
            </div>
        );
    }
}