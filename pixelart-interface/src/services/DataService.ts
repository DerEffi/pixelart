import axios, { AxiosResponse, AxiosError } from "axios";
import { APIError, APIErrorType } from "../models/Errors";
import { Display, Images, Socials, Time, Wifi, IWifiNetwork } from "../models/Settings";
import { Status } from "../models/Status";
import { VersionDetails } from "../models/Version";

export default class DataService {

    private deviceAddress: string = process.env.REACT_APP_ENVIRONMENT !== "device" ? "" : window.location.host;
    private darkTheme: boolean = true;
    public dismissedWifiWarning: boolean = false;
    private status: Status = Status.pending;
    private apiKey: string | undefined;
    private authInterval: NodeJS.Timer | undefined;
    public onChanged: () => void;
    public data: {
        wifiScan: IWifiNetwork[],
        wifi?: Wifi,
        time?: Time,
        display?: Display,
        socials?: Socials,
        images?: Images,
    } = {
        wifiScan: []
    };
    public newestFirmware?: VersionDetails;
    public newestWebinterface?: VersionDetails;

    constructor(onChanged: () => void) {
        this.onChanged = onChanged;

        let theme: string | null = localStorage.getItem("theme");
        if(theme && theme === "light")
            this.darkTheme = false;

        let dismissedWifiWarning: string | null = localStorage.getItem("dismissedWifiWarning");
        console.log(dismissedWifiWarning);
        if(dismissedWifiWarning === "true")
            this.dismissedWifiWarning = true;

        if(process.env.REACT_APP_ENVIRONMENT) {
            let deviceAddress: string | null = localStorage.getItem("deviceAddress");
            if(deviceAddress && deviceAddress.length > 3)
                this.deviceAddress = deviceAddress;
        }

        let apiKey: string | null = localStorage.getItem("apiKey");
        if(apiKey)
            this.apiKey = apiKey;

        this.checkUpdateVersions();
        this.refresh();
    }

    public async checkUpdateVersions() {
        axios.get(process.env.REACT_APP_UPDATE_SERVER + "/webinterface/version.json")
        .then(resp => {
            if(resp.status === 200 && resp.data.version && resp.data.files)
                this.newestWebinterface = resp.data;
        })
        .catch(() => {});

        axios.get(process.env.REACT_APP_UPDATE_SERVER + "/firmware/version.json")
        .then(resp => {
            if(resp.status === 200 && resp.data.version && resp.data.files)
                this.newestFirmware = resp.data;
        })
        .catch(() => {});
    }

    public async dismissWifiWarning() {
        localStorage.setItem("dismissedWifiWarning", "true");
        this.dismissedWifiWarning = true;
    }

    public getDeviceAddress() {
        return this.deviceAddress;
    }
    public setDeviceAddress(address: string = window.location.host) {
        this.deviceAddress = address;
        if(process.env.REACT_APP_ENVIRONMENT !== "device")
            localStorage.setItem("deviceAddress", address);
        this.onChanged();
    }

    public isDarkTheme(): boolean { return this.darkTheme; }
    public setTheme(isDark: boolean) {
        this.darkTheme = isDark;
        localStorage.setItem("theme", isDark ? "dark" : "light");
        this.onChanged();
    }

    public getStatus() { return this.status; }
    private setStatus(status: Status) {
        this.status = status;
        if(this.status === Status.unauthorized && this.authInterval === undefined) {
            this.authInterval = setInterval(() => this.authenticateDevice(), 2500);
            this.authenticateDevice();
        }
        this.onChanged();
    }

    public async refresh(ignoreError: boolean = false) {
        await this.refreshDisplay(ignoreError);
        await this.refreshWifi(ignoreError);
        await this.refreshTime(ignoreError);
        await this.refreshSocials(ignoreError);
        await this.refreshImages(ignoreError);
    }

    public async refreshDisplay(ignoreError: boolean = false) {
        await this.requestDevice<Display>("GET", "/api/display")
            .then(resp => {
                this.data.display = resp
            })
            .catch((e: APIError) => {
                if(e.type !== APIErrorType.UnauthorizedError && e.type !== APIErrorType.AuthPending && !ignoreError)
                    this.setStatus(Status.disconnected);
            });
        this.onChanged();
    }

    public async refreshWifi(ignoreError: boolean = false) {
        await this.requestDevice<Wifi>("GET", "/api/wifi")
            .then(resp => {
                this.data.wifi = resp
            })
            .catch((e: APIError) => {
                if(e.type !== APIErrorType.UnauthorizedError && e.type !== APIErrorType.AuthPending && !ignoreError)
                    this.setStatus(Status.disconnected);
            });
        this.onChanged();
    }

    public async refreshTime(ignoreError: boolean = false) {
        await this.requestDevice<Time>("GET", "/api/time")
            .then(resp => {
                this.data.time = resp
            })
            .catch((e: APIError) => {
                if(e.type !== APIErrorType.UnauthorizedError && e.type !== APIErrorType.AuthPending && !ignoreError)
                    this.setStatus(Status.disconnected);
            });
        this.onChanged();
    }

    public async refreshSocials(ignoreError: boolean = false) {
        await this.requestDevice<Socials>("GET", "/api/socials")
            .then(resp => {
                this.data.socials = resp
            })
            .catch((e: APIError) => {
                if(e.type !== APIErrorType.UnauthorizedError && e.type !== APIErrorType.AuthPending && !ignoreError)
                    this.setStatus(Status.disconnected);
            });
        this.onChanged();
    }
    
    public async refreshImages(ignoreError: boolean = false) {
        await this.requestDevice<Images>("GET", "/api/images")
            .then(resp => {
                this.data.images = resp
            })
            .catch((e: APIError) => {
                if(e.type !== APIErrorType.UnauthorizedError && e.type !== APIErrorType.AuthPending && !ignoreError)
                    this.setStatus(Status.disconnected);
            });
        this.onChanged();
    }

    public async scanWifi(): Promise<IWifiNetwork[]> {
        let networks = await this.requestDevice<{networks: IWifiNetwork[]}>("GET", "/api/wifi/available")
            .then(resp => {
                if(resp.networks)
                    return resp.networks
                return [];
            })
            .catch(() => []);
        
        //Sort by strength and remove duplicates
        this.data.wifiScan = networks
            .sort((a, b) => b.rssi - a.rssi)
            .filter((network, index, array) => 
                index === array.findIndex(n =>
                    n.ssid === network.ssid
                )
            );

        return this.data.wifiScan;
    }

    public async requestDevice<ResponseType>(method: string, endpoint: string, data: any = null): Promise<ResponseType> {
        if(!this.deviceAddress || this.deviceAddress.length < 5)
            throw new APIError(APIErrorType.DeviceAddressError, "Please enter your device's address");

        if(this.authInterval !== undefined)
            throw new APIError(APIErrorType.AuthPending, "Please press the menu button on your device to authorize");

        return axios.request({
            method: method,
            url: "http://" + this.deviceAddress + endpoint,
            headers: {
                "apiKey": this.apiKey
            },
            data: data
        }).then((resp: AxiosResponse<any, any>) => {
            if(resp.status !== 200) {
                throw new APIError(APIErrorType.ConnectionError, "Error requesting data from your device");
            } else {
                this.setStatus(Status.connected);
                return resp.data as ResponseType;
            }
        }).catch((e: AxiosError) => {
            if(e.response?.status === 403) {
                this.setStatus(Status.unauthorized);
                throw new APIError(APIErrorType.UnauthorizedError, "Please authorize your device");
            } else {
                throw new APIError(APIErrorType.ConnectionError, "Error requesting data from your device");
            }
        });
    }

    private async authenticateDevice(): Promise<void> {
        if(!this.deviceAddress || this.deviceAddress.length < 5)
            return;
        
        if(this.status === Status.connected) {
            if(this.authInterval)
                clearInterval(this.authInterval);
            this.authInterval = undefined;
        } else {
            axios.request({
                method: "GET",
                url: "http://" + this.deviceAddress + "/api/apiKey"
            }).then((resp: AxiosResponse<any, any>) => {
                if(resp.status === 200 && resp.data && resp.data.apiKey) {
                    clearInterval(this.authInterval);
                    this.apiKey = resp.data.apiKey;
                    localStorage.setItem("apiKey", resp.data.apiKey);
                    this.authInterval = undefined;
                    this.refresh();
                    this.setStatus(Status.connected);
                } else if(resp.status !== 204) {
                    clearInterval(this.authInterval);
                    this.authInterval = undefined;
                    this.setStatus(Status.disconnected);
                } else {
                    this.setStatus(Status.unauthorized);
                }
            }).catch((e: AxiosError) => {
                this.setStatus(Status.disconnected);
            });
        }
    }

}