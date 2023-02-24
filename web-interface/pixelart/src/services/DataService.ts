export default class DataService {

    private deviceAddress: string = process.env.REACT_APP_ENVIRONMENT !== "device" ? "" : window.location.host;
    private darkTheme: boolean = true;

    constructor() {
        let theme: string | null = localStorage.getItem("theme");
        if(theme && theme === "light")
            this.darkTheme = false;

        if(process.env.REACT_APP_ENVIRONMENT) {
            let deviceAddress: string | null = localStorage.getItem("deviceAddress");
            if(deviceAddress && deviceAddress.length > 3)
                this.deviceAddress = deviceAddress;
        }
    }

    public getDeviceAddress() {
        return this.deviceAddress;
    }
    public setDeviceAddress(address: string = window.location.host) {
        this.deviceAddress = address;
        if(process.env.REACT_APP_ENVIRONMENT !== "device")
            localStorage.setItem("deviceAddress", address);
    }

    public isDarkTheme(): boolean { return this.darkTheme; }
    public setTheme(isDark: boolean) {
        this.darkTheme = isDark;
        localStorage.setItem("theme", isDark ? "dark" : "light");
    }

}