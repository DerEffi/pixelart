export interface Wifi {
	wifiAP: boolean;
	wifiAPSSID: string;
	wifiAPPassword: string;
	wifiAPIP: string;
	wifiConnect: boolean;
	wifiSetupComplete: boolean;
	wifiSSID: string;
	wifiIP: string;
	wifiHostname: string;
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
}