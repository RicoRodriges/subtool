import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import 'bootstrap/dist/css/bootstrap.min.css';
import App from './App';
import SubtitleStore from "./stores/subtitle-store";

const subtitleStore = new SubtitleStore();
const secondSubtitle = new SubtitleStore();
const stores = {
    subtitleStore, secondSubtitle
};
type TStore = typeof stores;

const storeContext = React.createContext<TStore>(stores);
export const useStore = () => {
    return React.useContext(storeContext);
}

ReactDOM.render(
    <React.StrictMode>
        <App/>
    </React.StrictMode>,
    document.getElementById('root')
);
