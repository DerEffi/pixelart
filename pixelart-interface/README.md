# <p align="center">Pixelart - Interface</p>

## About

This React App is used to remote control the device controller by browser.

## Installing

1. Install [NodeJS](https://nodejs.org) on your system. This app waas created with version 17. Although different versions shouldn't be a problem, I can't guarantee that it will work. If you have any trouble try switching to version 17.
1. Install dependencies by running `npm install` in the project directory.

## Customization

You can change the settings in the `.env` file:
- `REACT_APP_ENVIRONMENT`:
    - `web` allows you to input a device ip. This is for hosted scenario like on my project page [pixelart.dereffi.de](http://pixelart.dereffi.de).
    - `device` will use the current domain and assumes it is hosted on the device. By using this option you can copy the build files to the `webinterface` directory on your sd-card
- `REACT_APP_VERSION`: Will compare to the Version present on the update server to determine if there is an update available
- `REACT_APP_UPDATE_SERVER`: You can edit the update server url if you want to use your own update server. By executing `scripts/versionInformation.ps1` you will get a `version.json` file to be placed inside `www.your-update.server/webinterface/` as a file list and version metadata.

## Running the App

In the project directory, you can run:

### `npm start`

Runs the app in the development mode.\
Open [http://localhost:3000](http://localhost:3000) to view it in the browser.

The page will reload if you make edits.\
You will also see any lint errors in the console.

### `npm run build`

Builds the app for production to the `build` folder.\
It correctly bundles React in production mode and optimizes the build for the best performance.

The build is minified and the filenames include the hashes.\
Your app is ready to be deployed!

See the section about [deployment](https://facebook.github.io/create-react-app/docs/deployment) for more information.

## Usage

Once you run the App you can find more details on how to use on the startpage or under [http://pixelart.dereffi.de](http://pixelart.dereffi.de).