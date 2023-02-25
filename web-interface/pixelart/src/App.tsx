import './App.scss';
import Generator from './components/pages/Generator';
import Home from './components/pages/Home';
import NotFound from './components/pages/NotFound';
import Footer from './components/shared/Footer';
import Sidebar from './components/shared/Sidebar';
import { ISidebarItem } from './models/SidebarItem';
import DataService from './services/DataService';
import 'primeicons/primeicons.css';
import PrimeReact from 'primereact/api';
import { InputText } from 'primereact/inputtext';
import 'primereact/resources/primereact.min.css';
import { Tooltip } from 'primereact/tooltip';
import React from 'react';
import { BiImages, BiRefresh, BiSun, BiTime, BiWifi } from 'react-icons/bi';
import { BsGearFill, BsHouseFill } from 'react-icons/bs';
import { CgMenu } from 'react-icons/cg';
import { IoShareSocialSharp } from 'react-icons/io5';
import { TfiLayoutGrid4Alt } from 'react-icons/tfi';
import { Route, Routes } from 'react-router-dom';
import Settings from './components/pages/Settings';
import { Toast } from 'primereact/toast';

PrimeReact.inputStyle = 'filled';

interface IAppState {
	loaded: boolean;
	sidebar: boolean;
	dark: boolean;
}

export default class App extends React.Component<{}, IAppState> {

	private dataService: DataService = new DataService(() => {this.setState({});});
	private toast: Toast | null = null;

	constructor(props: {}) {
		super(props);

		this.state = {
			loaded: true,
			sidebar: true,
			dark: true
		}
	}

	componentDidMount(): void {
		this.changeTheme(this.dataService.isDarkTheme());

		if(window.screen.width < 769)
			this.changeSidebar(false);
	}

	public render(): JSX.Element {
		return(
			<div id='fullpage'>
				<div className="sidebar-topbar">
					<div>
						<div>
							<Toast ref={(el) => this.toast = el} />
							<Tooltip target=".sidebar-topbar-item" position='bottom' showDelay={250} />
							<div onClick={() => this.changeSidebar()} className="sidebar-topbar-item">
								<CgMenu/>
							</div>
							<div onClick={() => this.changeTheme()} className="sidebar-topbar-item" data-pr-tooltip="Theme">
								<BiSun/>
							</div>
							{process.env.REACT_APP_ENVIRONMENT !== "device" &&
								<InputText className="sidebar-topbar-item sidebar-topbar-input" data-pr-tooltip="Hostname/IP-Address" placeholder='device address' value={this.dataService.getDeviceAddress()} onChange={(e) => { this.dataService.setDeviceAddress(e.target.value)}} />
							}
							<div onClick={() => this.dataService.refresh()} className="sidebar-topbar-item" data-pr-tooltip="Refresh">
								<BiRefresh/>
							</div>
						</div>
					</div>
				</div>
				<div id='fullpage-content'>
					<div id="sidebar" className={this.state.sidebar ? "sidebar-visible" : "sidebar-hidden"}>
						<Sidebar model={SidebarMenu} onLink={() => {if(window.screen.width < 769) {this.changeSidebar(false);}}} />
					</div>
					<div id="sidebar-layout" className={this.state.sidebar ? "sidebar-layout-sidebar" : "sidebar-layout-full"}>
						
						<div id="sidebar-block" onClick={() => this.changeSidebar() } />
						<Routes>
							<Route path="/settings/*" element={<Settings dataService={this.dataService} toast={this.toast} />} />
							<Route path="/pictures" element={<Generator dataService={this.dataService} toast={this.toast} />} />
							<Route path="/" element={<Home dataService={this.dataService} toast={this.toast} />} />
							<Route path="*" element={<NotFound/>} />
						</Routes>
					</div>
				</div>
				<Footer/>
			</div>
		);
	}

	private changeSidebar(active: boolean = !this.state.sidebar) {
		this.setState({
			sidebar: active
		});
	}

	private changeTheme(dark: boolean = !this.state.dark) {
		let element = document.getElementById('theme-css');
        if(element) {
            let href = element.getAttribute("href");
            if(href) {
                href = href.substring(0, href.lastIndexOf("/"));
                element.setAttribute("href", href + "/" + (dark ? "dark.css" : "light.css"));
				this.dataService.setTheme(dark);
				this.setState({
					dark: dark
				});
				return;
            }
        }
		console.error("Could not set theme properly - Missing html attributes");
	}

}

const SidebarMenu: ISidebarItem[] = [
    {
        label: 'Features',
        items: [
            {
                label: 'Home',
                url: '/',
				icon: <BsHouseFill/>
            },
			{
                label: 'Pictures',
                url: '/pictures',
				icon: <BiImages/>
            }
        ]
    },
    {
        label: 'Settings',
        items: [
			{
                label: 'Display',
                url: '/settings/display',
				icon: <TfiLayoutGrid4Alt/>
            },
			{
                label: 'Pictures',
                url: '/settings/pictures',
				icon: <BiImages/>
            },
			{
                label: 'Time',
                url: '/settings/time',
				icon: <BiTime/>
            },
			{
                label: 'WiFi',
                url: '/settings/wifi',
				icon: <BiWifi/>
            },
			{
                label: 'Socials',
                url: '/settings/socials',
				icon: <IoShareSocialSharp/>
            },
			{
                label: 'System',
                url: '/settings/system',
				icon: <BsGearFill/>
            },
        ]
    },
];