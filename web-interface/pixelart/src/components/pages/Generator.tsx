import DataService from '../../services/DataService';
import React from 'react';

export interface IGeneratorComponentProps {
	dataService: DataService;
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