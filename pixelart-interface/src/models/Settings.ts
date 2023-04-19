//device api interfaces

export interface Wifi {
	ap: boolean;
	apSSID: string;
	apPassword: string;
	apIP: string;
	connect: boolean;
	setupComplete: boolean;
	ssid: string;
	ip: string;
	hostname: string;
	rssi: number;
	mac: number;
}

export interface IWifiNetwork {
	ssid: string;
	rssi: number;
	encryption: number;
}

export interface Time {
	online: number;
	time: number;
	displayMode: number;
	seconds: boolean;
	year: boolean;
	blink: boolean;
	format24: boolean;
	externalRTCConnected: boolean;
	updateTime: boolean;
	ntpServer: string;
	timezone: string;
}

export interface Display {
	displayMode: number;
	brightness: number;
	version: string;
	freeMemory: number;
	freeSPIMemory: number;
	refreshRate: number;
	animation: boolean;
	animationTime: number;
	diashow: boolean;
	diashowTime: number;
	diashowModes: boolean;
}

export interface Images {
	displayImage: number;
	imageNumber: number;
	imagePrefixMax: number;
	imageLoaded: boolean;
	images: IImageData[];
}

export interface IImageData {
	prefix: number;
	folder: string;
	animated: boolean;
	rename?: string;
	delete?: boolean;
}

export interface IImageOperation {
	src: string;
	dst?: string
}

export interface Socials {
	request: string;
	server: string;
	apiKey: string;
	displayChannel: number;
	channelNumber: number;
}