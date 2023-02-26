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
import moment from 'moment';

export interface ITimeSettingsComponentProps {
	dataService: DataService;
	toast: Toast | null;
}

interface ITimeSettingsComponentState {
	ntpServer: string;
	ntpServerChanged: boolean;
	timezone: string;
	deviceTime?: Date;
	deviceTimeChanged: boolean;
	deviceTimePulled: boolean;
}

export default class TimeSettings extends React.Component<ITimeSettingsComponentProps, ITimeSettingsComponentState> {

	constructor(props: ITimeSettingsComponentProps) {
		super(props);

		this.state = {
			ntpServer: this.props.dataService.data.time?.ntpServer || "",
			ntpServerChanged: false,
			timezone: localStorage.getItem("selectedTimezone") || "",
			deviceTimeChanged: false,
			deviceTimePulled: false
		}
	}

	componentDidUpdate(prevProps: Readonly<ITimeSettingsComponentProps>, prevState: Readonly<ITimeSettingsComponentState>, snapshot?: any): void {
		if(!this.state.ntpServerChanged && this.props.dataService.data.time && this.state.ntpServer !== this.props.dataService.data.time.ntpServer) {
			this.setState({
				ntpServer: this.props.dataService.data.time.ntpServer
			});
		}

		if(this.state.timezone && this.props.dataService.data.time && this.props.dataService.data.time.timezone !== this.state.timezone.substring(this.state.timezone.indexOf("%") + 1, this.state.timezone.length)) {
			this.setState({timezone: ""});
		}

		if(!this.state.deviceTimeChanged && !this.state.deviceTimePulled && this.props.dataService.data.time) {
			this.setState({deviceTimePulled: true, deviceTime: moment.unix(this.props.dataService.data.time.time).toDate()});
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
							<Calendar className='input-group-field' value={this.state.deviceTime} onChange={(e) => this.setState({deviceTimeChanged: true, deviceTime: e.target.value ? e.target.value as Date : undefined})} showTime hourFormat={this.props.dataService.data.time && !this.props.dataService.data.time.format24 ? "12" : "24"} dateFormat="dd.mm.yy" />
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.deviceTimeChanged || !this.state.deviceTime} onClick={() => this.setDeviceTime(this.state.deviceTime)} />
						</div>
					</div>

					<div>
						<div className='headline'>Time Server</div>
						<div className='input-group'>
							<InputText className='input-group-field' value={this.state.ntpServer} onChange={(e) => this.setState({ntpServerChanged: true, ntpServer: e.target.value})} />
							<Button icon="pi pi-check" style={{borderRadius: "100%"}} disabled={!this.state.ntpServerChanged} onClick={() => this.setNtpServer(this.state.ntpServer)} />
						</div>
					</div>

					<div>
						<div className='headline'>Timezone</div>
						<div className='input-group'>
							<Dropdown className='input-group-field' value={this.state.timezone} filter options={timezoneOptions} onChange={(e) => this.setTimezone(e.target.value)} optionLabel="name" optionGroupLabel="label" optionGroupChildren="items"/>
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
								<td>Show Blinking Dots</td>
								<td><Checkbox checked={this.props.dataService.data.time?.blink || false} onChange={(e) => this.setTimeData({blink: e.target.checked})} /></td>
							</tr>
							<tr>
								<td>Show 24h Format</td>
								<td><Checkbox checked={this.props.dataService.data.time?.format24 || false} onChange={(e) => this.setTimeData({format24: e.target.checked})} /></td>
							</tr>
						</tbody>
					</table>
				</div>
            </div>
        );
    }

	public setTimeData(data: any) {
        this.props.dataService.requestDevice("POST", "/api/time", data)
			.then(() => this.props.dataService.refresh().then(() => this.setState({deviceTimePulled: false})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	public setNtpServer(server: string) {
        this.props.dataService.requestDevice("POST", "/api/time", {ntpServer: server})
			.then(() => this.props.dataService.refresh().then(() => this.setState({ntpServerChanged: false, deviceTimePulled: false})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	public setDeviceTime(time: Date | undefined) {
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
			.then(() => this.props.dataService.refresh().then(() => this.setState({deviceTimeChanged: false, deviceTimePulled: false})))
			.catch((e: APIError) => {
				if(this.props.toast)
					this.props.toast.show({
						content: e.message,
						severity: 'error',
						closable: false
					});
			});
    }

	public setTimezone(timezone: string) {
		localStorage.setItem("selectedTimezone", timezone);
		this.props.dataService.requestDevice("POST", "/api/time", {timezone: timezone.substring(timezone.indexOf("%") + 1, timezone.length)})
			.then(() => this.props.dataService.refresh().then(() => this.setState({timezone: timezone, deviceTimePulled: false})))
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