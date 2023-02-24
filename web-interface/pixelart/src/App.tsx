import React from 'react';
import './App.scss';
import "primereact/resources/primereact.min.css";
import "primeicons/primeicons.css";
import PrimeReact from 'primereact/api';
import Footer from './components/shared/Footer';
import Sidebar from "./components/shared/Sidebar";
import DataService from './services/DataService';
import NotFound from './components/pages/NotFound';
import Generator from './components/pages/Generator';
import { Routes, Route } from 'react-router-dom';
import { ISidebarItem } from './models/SidebarItem';
import { CgMenu } from 'react-icons/cg';
import { BiDownload, BiImages, BiSun, BiTime, BiWifi } from 'react-icons/bi';
import { BsHouseFill } from 'react-icons/bs';
import { IoShareSocialSharp } from 'react-icons/io5';
import { TfiLayoutGrid4Alt } from 'react-icons/tfi';
import Home from './components/pages/Home';
import { InputText } from 'primereact/inputtext';


PrimeReact.inputStyle = 'filled';

interface IAppState {
	loaded: boolean;
	sidebar: boolean;
	dark: boolean;
	deviceAddress: string;
}

export default class App extends React.Component<{}, IAppState> {

	private dataService: DataService = new DataService();

	constructor(props: {}) {
		super(props);

		this.state = {
			loaded: true,
			sidebar: true,
			dark: true,
			deviceAddress: this.dataService.getDeviceAddress()
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
					<div onClick={() => this.changeSidebar()} className="sidebar-topbar-item">
						<CgMenu/>
					</div>
					<div onClick={() => this.changeTheme()} className="sidebar-topbar-item">
						<BiSun/>
					</div>
					{process.env.REACT_APP_ENVIRONMENT !== "device" &&
						<InputText className="sidebar-topbar-item sidebar-topbar-input"  placeholder='device address' value={this.dataService.getDeviceAddress()} onChange={(e) => { this.setState({deviceAddress: e.target.value}); this.dataService.setDeviceAddress(e.target.value)}} />
					}
				</div>
				<div id='fullpage-content'>
					<div id="sidebar" className={this.state.sidebar ? "" : "sidebar-hidden"}>
						<Sidebar model={SidebarMenu} />
					</div>
					<div id="sidebar-layout" className={this.state.sidebar ? "" : "sidebar-layout-full"}>
						<Routes>
							<Route path="/" element={<Home dataService={this.dataService} />} />
							<Route path="/images" element={<Generator dataService={this.dataService} />} />
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
                label: 'Images',
                url: '/images',
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
                label: 'Images',
                url: '/settings/images',
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
                label: 'Updates',
                url: '/settings/updates',
				icon: <BiDownload/>
            },
        ]
    },
];