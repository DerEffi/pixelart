import DataService from '../../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { APIError } from '../../../models/Errors';
import { BiTime } from 'react-icons/bi';
import { IoMdCalendar } from 'react-icons/io';
import { HiOutlineViewGrid } from 'react-icons/hi';
import { CgDisplayFullwidth } from 'react-icons/cg';
import { Button } from 'primereact/button';
import { InputText } from 'primereact/inputtext';
import { Dropdown } from 'primereact/dropdown';
import { timezoneOptions } from '../../shared/Timezones';
import { Checkbox } from 'primereact/checkbox';
import { Calendar } from 'primereact/calendar';
import { InputNumber } from 'primereact/inputnumber';
import { InputSwitch } from 'primereact/inputswitch';
import moment from 'moment';

export interface ITimeSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
	advanced: boolean;
}

interface ITimeSettingsComponentState {
	ntpServer?: string;
	timezone: string;
	timezoneChanged: boolean;
	deviceTime?: Date;
}

export default class TimeSettings extends React.Component<ITimeSettingsComponentProps, ITimeSettingsComponentState> {

	constructor(props: ITimeSettingsComponentProps) {
		super(props);

		this.state = {
			timezone: localStorage.getItem("selectedTimezone") || "",
			timezoneChanged: false,
		}
	}

	componentDidUpdate(prevProps: Readonly<ITimeSettingsComponentProps>, prevState: Readonly<ITimeSettingsComponentState>, snapshot?: any): void {
		if(!this.state.timezoneChanged && this.state.timezone && this.props.dataService.data.time && this.props.dataService.data.time.timezone !== this.state.timezone.substring(this.state.timezone.indexOf("%") + 1, this.state.timezone.length)) {
			this.setState({timezone: ""});
		}
	}

    public render() {
        return(
            <div className='fullwidth'>
				<div className="content centered">

					<div>
						<div className='headline'>Display Mode</div>
						<div className='radio-container'>
							<div onClick={() => this.setTimeData({displayMode: 0}) } className={ this.props.dataService.data.time?.displayMode === 0 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<BiTime/>
								</div>
								<div className='radio-container-text'>Analog</div>
							</div>
							<div onClick={() => this.setTimeData({displayMode: 1}) } className={ this.props.dataService.data.time?.displayMode === 1 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<CgDisplayFullwidth/>
								</div>
								<div className='radio-container-text'>Digital</div>
							</div>
							<div onClick={() => this.setTimeData({displayMode: 2}) } className={ this.props.dataService.data.time?.displayMode === 2 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<IoMdCalendar/>
								</div>
								<div className='radio-container-text'>Date</div>
							</div>
							<div onClick={() => this.setTimeData({displayMode: 3}) } className={ this.props.dataService.data.time?.displayMode === 3 ? "selected" : ""}>
								<div className='radio-container-icon'>
									<HiOutlineViewGrid/>
								</div>
								<div className='radio-container-text'>Big</div>
							</div>
						</div>
					</div>

					<div>
						<div className='headline'>Device Time</div>
						<div className='input-group'>
							{!this.props.advanced &&
								<Calendar className='input-group-field' value={!this.state.deviceTime && this.props.dataService.data.time ? moment.unix(this.props.dataService.data.time.time).toDate() : this.state.deviceTime} onChange={(e) => this.setState({deviceTime: e.target.value ? e.target.value as Date : undefined})} showTime hourFormat={this.props.dataService.data.time && !this.props.dataService.data.time.format24 ? "12" : "24"} dateFormat="dd.mm.yy" />
							}
							{this.props.advanced &&
								<InputNumber format={false} className='input-group-field' value={!this.state.deviceTime && this.props.dataService.data.time ? this.props.dataService.data.time.time : (this.state.deviceTime ? Math.floor(this.state.deviceTime.getTime() / 1000) : 0)} onChange={(e) => this.setState({deviceTime: moment.unix(e.value ? e.value : 0).toDate()})} />
							}
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.deviceTime} onClick={() => this.setDeviceTime(this.state.deviceTime)} />
						</div>
					</div>

					{this.props.advanced &&
						<div>
							<div className='headline'>Time Server</div>
							<div className='input-group'>
								<InputText className='input-group-field' value={!this.state.ntpServer && this.props.dataService.data.time ? this.props.dataService.data.time.ntpServer : this.state.ntpServer} onChange={(e) => this.setState({ntpServer: e.target.value})} />
								<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.ntpServer} onClick={() => this.setNtpServer(this.state.ntpServer || "")} />
							</div>
						</div>
					}

					<div>
						<div className='headline'>Timezone</div>
						<div className='input-group'>
							{!this.props.advanced &&
								<Dropdown className='input-group-field' value={this.state.timezone} filter options={timezoneOptions} onChange={(e) => this.setTimezone(e.target.value)} optionLabel="name" optionGroupLabel="label" optionGroupChildren="items"/>
							}
							{this.props.advanced &&
								<>
									<InputText className='input-group-field' value={!this.state.timezoneChanged && this.props.dataService.data.time ? this.props.dataService.data.time.timezone : this.state.timezone} onChange={(e) => this.setState({timezoneChanged: true, timezone: e.target.value})} />
									<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.timezoneChanged} onClick={() => this.setTimezone(this.state.timezone)} />
								</>
							}
						</div>
					</div>

					<table className='table-group'>
						<tbody>
							<tr>
								<td>Auto update time</td>
								<td><Checkbox checked={this.props.dataService.data.time?.updateTime || false} onChange={(e) => this.setTimeData({updateTime: e.target.checked})} /></td>
							</tr>
							<tr>
								<td>Show Years</td>
								<td><Checkbox checked={this.props.dataService.data.time?.year || false} onChange={(e) => this.setTimeData({year: e.target.checked})} /></td>
							</tr>
							<tr>
								<td>Show Seconds</td>
								<td><Checkbox checked={this.props.dataService.data.time?.seconds || false} onChange={(e) => this.setTimeData({seconds: e.target.checked})} /></td>
							</tr>
							<tr>
								<td>Blinking Dots</td>
								<td><Checkbox checked={this.props.dataService.data.time?.blink || false} onChange={(e) => this.setTimeData({blink: e.target.checked})} /></td>
							</tr>
							<tr>
								<td>{this.props.dataService.data.time?.format24 ? "24" : "12"}h Format</td>
								<td><InputSwitch checked={this.props.dataService.data.time?.format24 || false} onChange={(e) => this.setTimeData({format24: e.value})} /></td>
							</tr>
						</tbody>
					</table>
				</div>
            </div>
        );
    }

	private setTimeData(data: any) {
        this.props.dataService.requestDevice("POST", "/api/time", data)
			.then(() => this.props.dataService.refreshTime())
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private setNtpServer(server: string) {
        this.props.dataService.requestDevice("POST", "/api/time", {ntpServer: server})
			.then(() => this.props.dataService.refreshTime().then(() => this.setState({ntpServer: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private setDeviceTime(time: Date | undefined) {
		if(!time) {
			if(this.props.toast)
			this.props.toast.show({
				content: "No time selected",
				severity: 'error',
				closable: false
			});
			return;
		}

        this.props.dataService.requestDevice("POST", "/api/time", {time: parseInt((time.getTime() / 1000).toFixed(0))})
			.then(() => this.props.dataService.refreshTime().then(() => this.setState({deviceTime: undefined})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	private setTimezone(timezone: string) {
		if(timezone.indexOf("%") !== -1)
			localStorage.setItem("selectedTimezone", timezone);
		else
			localStorage.removeItem("selectedTimezone");
		this.props.dataService.requestDevice("POST", "/api/time", {timezone: timezone.indexOf("%") !== -1 ? timezone.substring(timezone.indexOf("%") + 1, timezone.length) : timezone})
			.then(() => this.props.dataService.refreshTime().then(() => this.setState({timezone: timezone, timezoneChanged: false})))
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