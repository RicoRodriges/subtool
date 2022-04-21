import React from 'react';
import {Button, Card} from 'react-bootstrap';
import {observer} from 'mobx-react-lite';
import {downloadAsFile} from '../../utils/file';
import {BadSubtitle, Subtitle} from '../../stores/subtitle-store';
import SubtitlePreview from '../subtitle-preview';

function SingleSubtitleTab({subtitle, button}: { subtitle?: Subtitle | BadSubtitle | undefined, button: string }) {
    if (!subtitle?.isOk()) {
        return null;
    }

    const toSubRip = () => {
        downloadAsFile(subtitle.toSubRip(), "subtitle.srt");
    }

    return (
        <>
            <Card.Title>General subtitle information</Card.Title>
            <Card.Text>
                Type: {subtitle.type}<br/>
                FPS: {subtitle.fps || 'not defined'}<br/>
                Encoding: {subtitle.encoding}<br/>
                <br/>
                Number of units: {subtitle.lines.length}<br/>
                First unit: {subtitle.firstUnit}<br/>
                Last unit: {subtitle.lastUnit}<br/>
            </Card.Text>
            <Button variant="success" onClick={toSubRip}>{button}</Button>
            <SubtitlePreview lines={subtitle.lines} className="mt-4"/>
        </>
    );
}

export default observer(SingleSubtitleTab);