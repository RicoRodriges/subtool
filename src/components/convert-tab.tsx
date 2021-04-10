import React, {useRef, useState} from "react";
import {Card, Nav} from "react-bootstrap";
import {useStore} from "../index";
import {ReturnCode} from "../types";
import {observer} from "mobx-react";
import SingleSubtitleTab from "./convert-tab/single-subtitle-tab";
import SummarySubtitleTab from "./convert-tab/summary-subtitle-tab";
import SubtitleForm, {FormData} from "./subtitle-form";

function errorCodeToString(e?: ReturnCode) {
    switch (e) {
        case ReturnCode.ERROR:
        case ReturnCode.MALLOC_ERROR:
            return 'Something went wrong. Try another file :(';
        case ReturnCode.UNKNOWN_SUBTITLE_FORMAT:
            return 'Unknown subtitle format';
        case ReturnCode.FPS_NOT_SET:
            return 'FPS is required for this format of subtitle';
    }
    return undefined;
}

enum Tabs {
    FIRST_SUBTITLE = 'Subtitle 1',
    SECOND_SUBTITLE = 'Subtitle 2',
    SUMMARY = 'Summary'
}

function ConvertTab() {
    const {subtitleStore} = useStore();
    const [selectedTab, setSelectedTab] = useState(Tabs.FIRST_SUBTITLE);
    const [isProcessing, setProcessing] = useState(false);
    const formElement = useRef<HTMLDivElement>(null);
    const subtitleElement = useRef<HTMLDivElement>(null);

    const firstSubtitleError = errorCodeToString(subtitleStore.firstSubtitle.parseCode);
    const secondSubtitleError = errorCodeToString(subtitleStore.secondSubtitle.parseCode);

    const processFile = (d: FormData) => {
        setProcessing(true);

        const file2 = (d.secondSubtitle && d.secondSubtitle.file) || undefined;
        const fps2 = (d.secondSubtitle && d.secondSubtitle.fps) || undefined;
        const encoding2 = (d.secondSubtitle && d.secondSubtitle.encoding) || undefined;
        const speed2 = (d.secondSubtitle && d.secondSubtitle.speed) || undefined;

        subtitleStore.align = d.align;
        subtitleStore.loadSubtitles(d.firstSubtitle.file, d.firstSubtitle.encoding, d.firstSubtitle.fps, d.firstSubtitle.speed,
            file2, encoding2, fps2, speed2, d.synchronization)
            .then(
                () => {
                    subtitleElement.current && subtitleElement.current.scrollIntoView({block: "start", behavior: "smooth"});
                },
                () => {
                    formElement.current && formElement.current.scrollIntoView({block: "start", behavior: "smooth"});
                }
            )
            .finally(() => setProcessing(false));
    };

    const availableTabs = [
        {
            id: Tabs.FIRST_SUBTITLE,
            name: Tabs.FIRST_SUBTITLE,
            disabled: false,
            selected: selectedTab === Tabs.FIRST_SUBTITLE,
            content: () => (
                <SingleSubtitleTab subtitle={subtitleStore.firstSubtitle} button="Download first subtitle as SubRip"/>)
        },
        {
            id: Tabs.SECOND_SUBTITLE,
            name: Tabs.SECOND_SUBTITLE,
            disabled: !subtitleStore.secondSubtitle.loaded,
            selected: selectedTab === Tabs.SECOND_SUBTITLE,
            content: () => (
                <SingleSubtitleTab subtitle={subtitleStore.secondSubtitle}
                                   button="Download second subtitle as SubRip"/>)
        },
        {
            id: Tabs.SUMMARY,
            name: Tabs.SUMMARY,
            disabled: !subtitleStore.mergeLines,
            selected: selectedTab === Tabs.SUMMARY,
            content: () => (<SummarySubtitleTab store={subtitleStore}/>)
        },
    ].filter(t => !t.disabled);
    const tab = (availableTabs.find(t => t.selected) || availableTabs[0]);

    return (
        <>
            <div ref={formElement}>
                <SubtitleForm isProcessing={isProcessing} onSubmit={processFile}
                              firstSubtitleError={firstSubtitleError} secondSubtitleError={secondSubtitleError}/>
            </div>

            <div ref={subtitleElement}>
                {
                    (subtitleStore.firstSubtitle.loaded) && (
                        <Card>
                            <Card.Header>
                                <Nav variant="tabs" activeKey={tab.id}
                                     onSelect={(tab: string) => setSelectedTab(tab as Tabs)}>
                                    {
                                        availableTabs.map(t => (
                                            <Nav.Item>
                                                <Nav.Link eventKey={t.id}>{t.name}</Nav.Link>
                                            </Nav.Item>
                                        ))
                                    }
                                </Nav>
                            </Card.Header>
                            <Card.Body>
                                {tab.content()}
                            </Card.Body>
                        </Card>
                    )
                }
            </div>
        </>
    );
}

export default observer(ConvertTab);