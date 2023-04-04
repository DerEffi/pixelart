import ReactDOM from 'react-dom/client';
import App from './App';
import { HashRouter, useLocation } from 'react-router-dom';
import "primereact/resources/primereact.min.css";
import "primeicons/primeicons.css";

const RouterWrapper = () => <App location={useLocation()}/>;

const root = ReactDOM.createRoot(
  document.getElementById('root') as HTMLElement
);
root.render(
  <HashRouter>
    <RouterWrapper/>
  </HashRouter>
);