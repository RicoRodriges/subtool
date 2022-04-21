import React from 'react';
import {Navbar} from 'react-bootstrap';
import ConvertTab from './components/convert-tab';

function App() {
    return (
        <>
            <Navbar bg="dark" variant="dark">
                <div className="container">
                    <Navbar.Brand href="#">Subtool</Navbar.Brand>
                </div>
            </Navbar>
            <div className="container">
                <ConvertTab/>
            </div>
            <footer className="page-footer text-center">
                Powered by VLC and libass source codes
            </footer>
        </>
    );
}

export default App;
