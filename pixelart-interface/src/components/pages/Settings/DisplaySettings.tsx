import DataService from '../../../services/DataService';
import React from 'react';
import { BiImages, BiTime } from 'react-icons/bi';
import { IoShareSocialSharp } from 'react-icons/io5';
import { Toast } from 'primereact/toast';
import { APIError } from '../../../models/Errors';
import { Slider } from 'primereact/slider';
import { Button } from 'primereact/button';
import { Checkbox } from 'primereact/checkbox';
import { Tooltip } from 'primereact/tooltip';
import { InputNumber } from 'primereact/inputnumber';

export interface IDisplaySettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface IDisplaySettingsComponentState {
	brightness?: number;
	animationTime?: number;
	diashowTime?: number;
}

export default class DisplaySettings extends React.Component<IDisplaySettingsComponentProps, IDisplaySettingsComponentState> {

	constructor(props: IDisplaySettingsComponentProps) {
		super(props);

		this.state = {
		}
	}

    public render() {
        return(
            <div className='fullwidth'>
				<div className="content centered">

					<div>
						<div className='headline'>Display Mode</div>
						<div className='radio-container'>
							<div onClick={() => this.setDisplayData({displayMode: 0}) } className={ this.props.dataService.data.display?.displayMode === 0 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<BiImages/>
								</div>
								<div className='radio-container-text'>Pictures</div>
							</div>
							<div onClick={() => this.setDisplayData({displayMode: 1}) } className={ this.props.dataService.data.display?.displayMode === 1 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<BiTime/>
								</div>
								<div className='radio-container-text'>Clock</div>
							</div>
							<div onClick={() => this.setDisplayData({displayMode: 2}) } className={ this.props.dataService.data.display?.displayMode === 2 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<IoShareSocialSharp/>
								</div>
								<div className='radio-container-text'>Socials</div>
							</div>
						</div>
					</div>

					<div>
						<div className='headline'>Brightness</div>
						<div  className='input-group'>
							<Slider className='input-group-field' value={this.state.brightness || this.props.dataService.data.display?.brightness} step={8} min={16} max={248} onChange={(e) => this.setState({brightness: e.value as number})} />
							<div style={{width: "3rem"}}>{this.state.brightness || this.props.dataService.data.display?.brightness}</div>
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.brightness} onClick={() => this.setBrightness(this.state.brightness || this.props.dataService.data.display?.brightness || 24)} />
						</div>
					</div>

					<div>
						<div className='headline'>Diashow</div>
						<div  className='input-group'>
							{!this.props.advanced &&
								<>
									<Tooltip position='bottom' target=".imagetime-tooltip">Imagetime</Tooltip>
									<Slider className='input-group-field' value={60000 - (this.state.diashowTime || this.props.dataService.data.display?.diashowTime || 1000)} step={1000} min={0} max={59000} onChange={(e) => this.setState({diashowTime: 60000 - (e.value as number)})} />
									<div className="imagetime-tooltip" style={{width: "3rem"}}>{(this.state.diashowTime || this.props.dataService.data.display?.diashowTime || 1000) / 1000}s</div>
								</>
							}
							{this.props.advanced &&
								<>
									<InputNumber step={1000} format={false} className='input-group-field' value={(this.state.diashowTime || this.props.dataService.data.display?.diashowTime || 1000)} onChange={(e) => this.setState({diashowTime: e.value as number})} />ms
								</>
							}
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.diashowTime} onClick={() => this.setDiashowTime(this.state.diashowTime || this.props.dataService.data.display?.diashowTime || 1000)} />
							<Button icon={this.props.dataService.data.display?.diashow ? "pi pi-pause" : "pi pi-play"} style={{borderRadius: "100%"}} onClick={() => this.setDisplayData({diashow: !this.props.dataService.data.display?.diashow || false})} />
						</div>
					</div>

					<div>
						<div className='headline'>Animation</div>
						<div  className='input-group'>
							{!this.props.advanced &&
								<>
									<Tooltip position='bottom' target=".frametime-tooltip">Frametime</Tooltip>
									<Slider className='input-group-field' value={500 - (this.state.animationTime || this.props.dataService.data.display?.animationTime || 20)} step={20} min={0} max={480} onChange={(e) => this.setState({animationTime: 500 - (e.value as number)})} />
									<div className="frametime-tooltip" style={{width: "3rem"}}>{(this.state.animationTime || this.props.dataService.data.display?.animationTime || 20)}ms</div>
								</>
							}
							{this.props.advanced &&
								<>
									<InputNumber step={20} format={false} className='input-group-field' value={(this.state.animationTime || this.props.dataService.data.display?.animationTime || 20)} onChange={(e) => this.setState({animationTime: e.value as number})} />ms
								</>
							}
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.animationTime} onClick={() => this.setAnimationTime(this.state.animationTime || this.props.dataService.data.display?.animationTime || 20)} />
							<Button icon={this.props.dataService.data.display?.animation ? "pi pi-pause" : "pi pi-play"} style={{borderRadius: "100%"}} onClick={() => this.setDisplayData({animation: !this.props.dataService.data.display?.animation || false})} />
						</div>
					</div>

					<table className='table-group'>
						<tbody>
							<tr>
								<td>Diashow between modes</td>
								<td><Checkbox checked={this.props.dataService.data.display?.diashowModes || false} onChange={(e) => this.setDisplayData({diashowModes: e.target.checked})} /></td>
							</tr>
						</tbody>
					</table>
				</div>
            </div>
        );
    }

	public setBrightness(val: number) {
		this.props.dataService.requestDevice("POST", "/api/display", {brightness: val})
			.then(() => this.props.dataService.refreshDisplay().then(() => this.setState({brightness: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
	}

	public setDiashowTime(val: number) {
		this.props.dataService.requestDevice("POST", "/api/display", {diashowTime: val, diashow: true})
			.then(() => this.props.dataService.refreshDisplay().then(() => this.setState({diashowTime: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
	}

	public setAnimationTime(val: number) {
		this.props.dataService.requestDevice("POST", "/api/display", {animationTime: val, animation: true})
			.then(() => this.props.dataService.refreshDisplay().then(() => this.setState({animationTime: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
	}

	public setDisplayData(data: any) {
        this.props.dataService.requestDevice("POST", "/api/display", data)
			.then(() => this.props.dataService.refreshDisplay())
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