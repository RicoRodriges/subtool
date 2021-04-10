import React from "react";
import {Accordion, Button, Card} from "react-bootstrap";
import {observer} from "mobx-react";
import {downloadAsFile} from "../../utils/file";
import SubtitleStore from "../../stores/subtitle-store";
import SubtitlePreview from "../subtitle-preview";

function SummarySubtitleTab({store}: { store: SubtitleStore }) {
    const mergeLines = store.mergeLines;
    if (!mergeLines) {
        return null;
    }

    const toSubRip = () => {
        const content = store.mergeToSubRip();
        if (content) {
            downloadAsFile(content, "subtitle.srt");
        }
    }

    const toASS = () => {
        const content = store.mergeToASS();
        if (content) {
            downloadAsFile(content, "subtitle.ass");
        }
    }

    return (
        <>
            Align: {store.align}
            <div className="my-3">
                <Button variant="success" onClick={toSubRip} className="mr-2">Download as SubRip</Button>
                <Button variant="success" onClick={toASS}>Download as ASS</Button>
            </div>
            <Accordion>
                <Accordion.Toggle as={Button} variant="link" eventKey="0" className="mt-3">
                    What is the difference?
                </Accordion.Toggle>
                <Accordion.Collapse eventKey="0">
                    <Card.Body>
                        <p>
                            <a href="https://wikipedia.org/wiki/SubRip">SubRip</a> is a very lightweight subtitle
                            format. This format is supported by most software video players but Duo mode is not a
                            standard feature and your subtitles may be rendered incorrect. We are recommend that you
                            use <a href="https://www.videolan.org/vlc/">VLC media player</a>.
                        </p>
                        <p>
                            <a href="https://wikipedia.org/wiki/SubStation_Alpha">ASS</a> is another subtitle format but
                            it's much complicated. Use this format if your player has some troubles with the SubRip
                            format.
                        </p>
                    </Card.Body>
                </Accordion.Collapse>
            </Accordion>
            <SubtitlePreview lines={mergeLines} className="mt-4"/>
        </>
    );
}

export default observer(SummarySubtitleTab);