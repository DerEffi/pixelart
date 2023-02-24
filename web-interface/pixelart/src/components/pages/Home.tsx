import DataService from '../../services/DataService';
import React from 'react';

export interface IHomeComponentProps {
	dataService: DataService;
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
					
                    

				</div>
				
            </div>
        );
    }
}