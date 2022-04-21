import React from 'react';
import {ToTimeString} from '../utils/time';
import {SubtitleLine} from '../stores/subtitle-store';
import {observer} from 'mobx-react-lite';
import {Accordion} from 'react-bootstrap';

function SubtitlePreview({lines, className}: { lines?: SubtitleLine[], className?: string }) {
    if (!lines) {
        return null;
    }
    return (
        <Accordion className={className}>
            <Accordion.Item eventKey="0">
                <Accordion.Header>
                    Preview
                </Accordion.Header>
                <Accordion.Body>
                    <table className="table table-hover table-sm">
                        <thead>
                        <tr>
                            <th scope="col">#</th>
                            <th scope="col">Start</th>
                            <th scope="col">End</th>
                            <th scope="col">Lines</th>
                        </tr>
                        </thead>
                        <tbody>
                        {
                            lines.map((l, i) => {
                                return (
                                    <tr>
                                        <th scope="row">{i + 1}</th>
                                        <td>{ToTimeString(l.start, ',')}</td>
                                        <td>{ToTimeString(l.stop, ',')}</td>
                                        <td style={{whiteSpace: 'pre-wrap'}}>{l.line}</td>
                                    </tr>
                                );
                            })
                        }
                        </tbody>
                    </table>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}

export default observer(SubtitlePreview);