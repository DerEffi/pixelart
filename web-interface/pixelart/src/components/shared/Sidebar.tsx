import React from "react";
import { Link } from "react-router-dom";
import { ISidebarItem } from "../../models/SidebarItem";

export interface ISidebarProps {
    model: ISidebarItem[];
    onLink?: () => void
}

export default class Sidebar extends React.Component<ISidebarProps, {}> {

    render() {
        return(
            <ul className="sidebar">
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