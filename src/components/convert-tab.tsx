import React, {useRef, useState} from 'react';
import {Card, Nav} from 'react-bootstrap';
import {useStore} from '../index';
import {observer} from 'mobx-react-lite';
import SingleSubtitleTab from './convert-tab/single-subtitle-tab';
import SummarySubtitleTab from './convert-tab/summary-subtitle-tab';
import SubtitleForm, {FormData} from './subtitle-form';
import {SubtitleFile} from '../stores/subtitle-store';
import {runInAction} from "mobx";

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

    const processFile = (d: FormData) => {
        setProcessing(true);

        const s1: SubtitleFile = {
            f: d.firstSubtitle.file,
            encoding: d.firstSubtitle.encoding,
            fps: d.firstSubtitle.fps,
            speed: d.firstSubtitle.speed,
            offset: 0,
        };

        const s2: SubtitleFile | undefined = !d.secondSubtitle ? undefined : {
            f: d.secondSubtitle.file,
            encoding: d.secondSubtitle.encoding,
            fps: d.secondSubtitle.fps,
            speed: d.secondSubtitle.speed,
            offset: d.offset,
        };

        runInAction(() => subtitleStore.align = d.align);
        subtitleStore.loadSubtitles(s1, s2)
            .then(
                () => subtitleElement.current?.scrollIntoView({block: "start", behavior: "smooth"}),
                () => formElement.current?.scrollIntoView({block: "start", behavior: "smooth"}),
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
                <SingleSubtitleTab subtitle={subtitleStore.firstSubtitle}
                                   button="Download first subtitle as SubRip"/>)
        },
        {
            id: Tabs.SECOND_SUBTITLE,
            name: Tabs.SECOND_SUBTITLE,
            disabled: !subtitleStore.secondSubtitle?.isOk(),
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
                              firstSubtitleValid={subtitleStore.firstSubtitle === undefined || subtitleStore.firstSubtitle.isOk()}
                              secondSubtitleValid={subtitleStore.secondSubtitle === undefined || subtitleStore.secondSubtitle.isOk()}/>
            </div>

            <div ref={subtitleElement} className="my-2">
                {
                    (subtitleStore.firstSubtitle?.isOk()) && (
                        <Card>
                            <Card.Header>
                                <Nav variant="tabs" activeKey={tab.id}
                                     onSelect={(tab: string | null) => setSelectedTab(tab as Tabs)}>
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