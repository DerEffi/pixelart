import DataService from '../../../services/DataService';
import React from 'react';
import { BiImages, BiTime } from 'react-icons/bi';
import { IoShareSocialSharp } from 'react-icons/io5';
import { Toast } from 'primereact/toast';
import { APIError } from '../../../models/Errors';
import { Slider } from 'primereact/slider';
import { Button } from 'primereact/button';

export interface IDisplaySettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface IDisplaySettingsComponentState {
	brightness: number;
	brightnessChanged: boolean;
}

export default class DisplaySettings extends React.Component<IDisplaySettingsComponentProps, IDisplaySettingsComponentState> {

	constructor(props: IDisplaySettingsComponentProps) {
		super(props);

		this.state = {
			brightness: this.props.dataService.data.display?.brightness || 24,
			brightnessChanged: false
		}
	}

	componentDidUpdate(prevProps: Readonly<IDisplaySettingsComponentProps>, prevState: Readonly<IDisplaySettingsComponentState>, snapshot?: any): void {
		if(!this.state.brightnessChanged && this.props.dataService.data.display && this.state.brightness !== this.props.dataService.data.display?.brightness) {
			this.setState({
				brightness: this.props.dataService.data.display.brightness
			});
		}
	}

    public render() {
        return(
            <div className='fullwidth'>
				<div className="content centered">

					<div>
						<div className='headline'>Display Mode</div>
						<div className='radio-container'>
							<div onClick={() => this.setDisplayMode(0) } className={ this.props.dataService.data.display?.displayMode === 0 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<BiImages/>
								</div>
								<div className='radio-container-text'>Pictures</div>
							</div>
							<div onClick={() => this.setDisplayMode(1) } className={ this.props.dataService.data.display?.displayMode === 1 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<BiTime/>
								</div>
								<div className='radio-container-text'>Clock</div>
							</div>
							<div onClick={() => this.setDisplayMode(2) } className={ this.props.dataService.data.display?.displayMode === 2 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<IoShareSocialSharp/>
								</div>
								<div className='radio-container-text'>Socials</div>
							</div>
						</div>
					</div>

					<div>
						<div className='headline'>Brightness</div>
						<div style={{display: "flex", alignItems: "center", gap: "1rem", justifyContent: "center", flexWrap: "wrap"}}>
							<Slider style={{width: "20rem", maxWidth: "70vw"}} value={this.state.brightness} step={8} min={16} max={248} onChange={(e) => this.setState({brightnessChanged: true, brightness: e.value as number})} />
							<div style={{width: "3rem"}}>{this.state.brightness}</div>
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.brightnessChanged} onClick={() => this.setBrightness(this.state.brightness)} />
						</div>
					</div>
				</div>
            </div>
        );
    }

	public setBrightness(val: number) {
		this.props.dataService.requestDevice("POST", "/api/display", {brightness: val})
			.then(() => this.props.dataService.refresh().then(() => this.setState({brightnessChanged: false})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
	}

	public setDisplayMode(mode: number) {
        this.props.dataService.requestDevice("POST", "/api/display", {displayMode: mode})
			.then(() => this.props.dataService.refresh())
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }
}