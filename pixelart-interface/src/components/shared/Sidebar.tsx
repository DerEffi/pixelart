import { InputText } from "primereact/inputtext";
import { Tooltip } from "primereact/tooltip";
import React from "react";
import { Link } from "react-router-dom";
import { ISidebarItem } from "../../models/SidebarItem";
import DataService from "../../services/DataService";

export interface ISidebarProps {
    model: ISidebarItem[];
    onLink?: () => void;
    dataService: DataService;
}

export default class Sidebar extends React.Component<ISidebarProps, {}> {

    render() {
        return(
            <ul className="sidebar">
                {process.env.REACT_APP_ENVIRONMENT !== "device" &&
                    <>
                        <Tooltip target=".sidebar-hostname" position='bottom' showDelay={250} />
                        <InputText className="sidebar-hostname" style={{width: "100%", marginBottom: "7px"}} data-pr-tooltip="Hostname/IP-Address" placeholder='device address' value={this.props.dataService.getDeviceAddress()} onChange={(e) => { this.props.dataService.setDeviceAddress(e.target.value)}} onBlur={() => this.props.dataService.refresh() } />
                    </>
                }
                {this.props.model.map(cat => {
                    return(
                        <li className="sidebar-category" role="none" key={cat.label}>
                            <div className="sidebar-category-label">
                                { cat.url &&
                                    <Link onClick={() => {if(this.props.onLink) { this.props.onLink()}}} to={cat.url}>{cat.icon && cat.icon} {cat.label}</Link>
                                }
                                { !cat.url &&
                                    <>{cat.icon && cat.icon} {cat.label}</>
                                }
                            </div>
                            <ul role="menu">
                                {cat.items.map(item => {
                                    return(
                                        <li className="sidebar-item" role="none" key={item.label}>
                                            <Link onClick={() => {if(this.props.onLink) { this.props.onLink()}}} to={item.url}>{item.icon && item.icon} {item.label}</Link>
                                        </li>
                                    );
                                })}
                            </ul>
                        </li>
                    );
                })}
            </ul>
        );
    }

}