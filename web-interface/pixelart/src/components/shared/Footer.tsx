import React from "react";
import { MdEmail } from "react-icons/md";
import { Link } from "react-router-dom";

export default class Footer extends React.Component<{}, {}> {

    public render() {
        return(
            <div id="footer" className="content">
                <div>
                    <span className="footer-item"><Link target={"_blank"} to="https://dereffi.de">&copy; DerEffi</Link></span>
                    <span className="footer-item"><Link target={"_blank"} to="https://dereffi.de/imprint">Imprint</Link></span>
                    <span className="footer-item"><Link target={"_blank"} to="https://dereffi.de/privacy">Privacy</Link></span>
                </div>
                <div>
                    <span className="footer-item">
                        <Link target={"_blank"} to="mailto:info@dereffi.de">
                            <span className="icon-text"><MdEmail/>&ensp;<span>Contact</span></span>
                        </Link>
                    </span>
                </div>
            </div>
        );
    }
}