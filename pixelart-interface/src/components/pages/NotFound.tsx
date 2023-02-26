import React from "react";
import { BsSearch } from "react-icons/bs";

export default class NotFound extends React.Component<{}, {}> {
    public render() {
        return(
            <div className="fullwidth full-centered">
                <span className="font-xxl primary">
                    <BsSearch/>
                </span>
                <br/>
                The page you are looking for, might not exist...
            </div>
        );
    }
}