import DataService from '../../services/DataService';
import React from 'react';
import { Toast } from 'primereact/toast';
import { Link, Location } from 'react-router-dom';
import { Fieldset } from 'primereact/fieldset';
import { BiImages, BiTime, BiUser, BiWifi } from 'react-icons/bi';
import { RxUpdate } from 'react-icons/rx';
import { IoShareSocialSharp, IoWarningOutline } from 'react-icons/io5';
import { scrollIntoView } from '../../services/Helper';
import { Chip } from 'primereact/chip';

export interface IHomeComponentProps {
	location: Location;
	dataService: DataService;
	toast: Toast | null;
}

interface IHomeComponentState {

}

export default class Home extends React.Component<IHomeComponentProps, IHomeComponentState> {

	constructor(props: IHomeComponentProps) {
		super(props);

		this.state = {
		}

		scrollIntoView(this.props.location.hash.replace("#", ""));
	}

	componentDidUpdate(prevProps: Readonly<IHomeComponentProps>, prevState: Readonly<IHomeComponentState>, snapshot?: any): void {
		if(prevProps.location.hash !== this.props.location.hash) {
			scrollIntoView(this.props.location.hash.replace("#", ""));
		}
	}

    public render() {
		console.log(this.props.location.hash);
        return(
            <div className='fullwidth'>

				<div className="content">
					
                    <h1 className='headline'>DerEffi's Pixelart</h1>

					<div>
						- Displaying Pixelart on an LED Matrix -
						<br/>
						<br/>
						This Webinterface provides the basic usage of the pixelart display as well as an Image Generator to convert your pictures into a for the display usable format. You can also remote control your device, if you choose to connect your display to a network or via Hotspot directly to your device. WiFi functionality is completely optional, although you can't use the 'Social' mode when not connected to the internet for obvious reasons.
						<br/>
						<br/>
						If you want to build a pixelart display yourself, take a look on <Link to="https://github.com/DerEffi/pixelart" target="_blank">GitHub</Link>. It is open source with requirements and build instructions waiting for you.
					</div>

					<Fieldset legend={<div className='home-header'><BiUser/><div>General</div></div>} id="general">

						You can control the device with the following buttons and knobs.

						<br/>
						<br/>

						<table className='home-table'>
							<tbody>
								<tr>
									<td>Next</td>
									<td>Switch to the next element in the current mode or menu</td>
								</tr>
								<tr>
									<td>Mode</td>
									<td>Switch to the next mode or change the selected menu item</td>
								</tr>
								<tr>
									<td>Menu</td>
									<td>Open/Close the menu</td>
								</tr>
								<tr>
									<td rowSpan={2}>Diashow</td>
									<td>Adjust the speed for switching through elements in and between modes</td>
								</tr>
								<tr>
									<td>(Press) Start/Stop the Diashow</td>
								</tr>
								<tr>
									<td rowSpan={2}>Animation</td>
									<td>Adjust the speed for switching through the frames of an image</td>
								</tr>
								<tr>
									<td>(Press) Start/Stop the Animation</td>
								</tr>
								<tr>
									<td>Brightness</td>
									<td>Adjust the brightness of the LED panel</td>
								</tr>
							</tbody>
						</table>

						<br/>

						These functions can also be controlled over the network once your display is <Link to="#wifi">connected</Link>, although it may take a second to apply the new preferences over the air.

					</Fieldset>

					<Fieldset legend={<div className='home-header'><BiImages/><div>Pictures</div></div>} id="pictures">
						The panel needs a special file format for the pictures you want to display. You can generate these files from images with the <Link to="/pictures">Picture Generator</Link>. The processed files can be downloaded in a zip and placed in the SD cards <Chip label='images'/> folder or directly uploaded if your display is <Link to="#wifi">connected to the network</Link>. If the folder doesn't exist, feel free to create one with the name <Chip label="images"/>.

						<br/>
						<br/>

						Notes on using the image generator:
						<table className='home-table'>
							<tbody>
								<tr>
									<td>Aspect ratio</td>
									<td>Use square images - Different aspect ratios will result in streched pixelarts</td>
								</tr>
								<tr>
									<td>Size</td>
									<td>Because of the LED panel size you cannot display more pixels. Although the generator resizes your image without any edge-interpolation you might loose some details, so for the best results you may want to manually edit your 64x64 image</td>
								</tr>
								<tr>
									<td>Color accuracy</td>
									<td>Because of the limited color-depth of the Display you might want to try experimenting with the colors in your image. FYI: The panel uses RGB-565 and displays in 5bit color depth, but due to the color correction of the individuals LEDs the representation on the display itself might not be always accurate.</td>
								</tr>
								<tr>
									<td>Animations</td>
									<td>Due to the limited memory big animations with lots of pixel changes might cause problems. If your device does not start or load the images properly you might want to try removing the animation from the SD card.</td>
								</tr>
							</tbody>
						</table>

						<br/>

						After uploading your image to the device, you can change the order on the <Link to="/settings/pictures">Pictures</Link> tab. You can also change the order by renaming the 3 digit prefix <Chip label='XXX - '/> of the folders on your sd card. The numbers dont have to be consecutive. They will be ordered on device startup from smallest to highest.
					</Fieldset>


					<Fieldset legend={<div className='home-header'><BiTime/><div>Time</div></div>} id="time">
						The Clock mode has different layouts you can switch through with the <Chip label="Next"/> button. For most layouts you can adjust preferences in the menu. If <Link to="#wifi">connected to the network</Link> you can also change the settings with the webinterface or update your time automatically over wifi. Depending on where you live you might want to adjust your timezone by activating the advanced mode in the webinterface.

						<br/>
						<br/>

						If your device doesn't save your time after turning it off, you might want to check your battery (CR2032) by opening the screws on the back of your panel.
					</Fieldset>


					<Fieldset legend={<div className='home-header'><BiWifi/><div>Wifi</div></div>} id="wifi">
						To connect your dispay to a network you first need to enable the <Chip label="Host"/> option in the onboard menu so that you can directly connect to the display itself with a browser of your choice. If you are unsure about the connection details, you can check the <Chip label='Host Stats'/> section for the current settings.
						<br/>
						<br/>
						After connecting to the display's network, you should be redirected to webinterface, where you can enter your network information. If not you can manually browse to <Link to='http://192.168.4.1' target="_blank">192.168.4.1</Link>.
						<br/>
						<br/>
						Make sure to deactivate the Host mode when you are finished setting up your device as it might occupy unnecessary resources you might rather want to use for a big animation... 
					</Fieldset>


					<Fieldset legend={<div className='home-header'><IoShareSocialSharp/><div>Socials</div></div>} id="socials">
						Since you need to <Link to="#wifi">connect the display to the network</Link> anyway, you can only adjust this mode on the webinterface. To set it up check the <Link to="/settings/system">System</Link> tab on the left.
						<br/>
						<br/>
						If connected to the network and after you added at least one social channel, this will show up, when you cycle through the modes on your display. You can also use the diashow as you are used to from your pictures and under <Link to="/settings/display">Display</Link> you can check the option to switch between pictures and socials automatically.
						<br/>
						<br/>
						Depending on the kind of data and source some stats might be cached for a short amount of time either by me, the display or the social media platform. As long as you stay connected to the network the stats get updated automatically, but you might see let's say your viewers number go up from a twitch raid one or two minutes later. 
					</Fieldset>


					<Fieldset legend={<div className='home-header'><RxUpdate/><div>Updates</div></div>} id="updates">
						In the <Link to="/downloads">Downloads</Link> tab you can download the newest version of the firmware and webinterface and place the files on the SD card. The update will be performed on the next start and should only take a few seconds. If you update the firmware, please don't disconnect the power from your device as it might brick the display.
						<br/>
						<br/>
						Notice: If you are or know a software developer you can also compile the firmware/webinterface from my <Link to="https://github.com/DerEffi/pixelart" target="_blank">GitHub</Link> with your personal touch yourself and copy the files onto the SD card.
					</Fieldset>


					<Fieldset legend={<div className='home-header'><IoWarningOutline/><div>Problems</div></div>} id="problems">
						If you have any problems with your device, sometimes a simple restart can fix it...
						<br/>
						<br/>
						Ocassonaly the display needs some time to connect to your Wifi network. If you were already connected to a network or know that your connection details are correct, please have a little patience. After a few minutes your device should get a connection. If not please check if your sign in information is correct.
						<br/>
						<br/>
						If your problem persist you can reset the device to "Factoy Settings" with the webinterface under <Link to="/settings/system">System</Link> or press and hold the <Chip label='Menu'/> button on your display for 10s.
						<br/>
						<br/>
						Still not fixed? - Sent me the details maybe you found a mistake I made (<Link to="mailto:info@dereffi.de">info@dereffi.de</Link>) or open a ticket on <Link to="https://github.com/DerEffi/pixelart/issues" target="_blank">GitHub</Link>
					</Fieldset>

				</div>
				
            </div>
        );
    }
}