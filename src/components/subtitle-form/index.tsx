import React, {useEffect} from "react";
import {Align} from "../../types";
import {observer, useLocalStore} from "mobx-react";
import {createFormValueStore} from "./form-value-store";
import {createFormControlStore} from "./form-control-store";
import {Alert, Button, ButtonGroup, Form, ToggleButton} from "react-bootstrap";
import AlignSVG from "../align-svg";
import InputControl from "./input-control";

export interface SubtitleData {
    file: File;
    fps?: number;
    encoding?: string;
    speed?: number;
}

export interface FormData {
    firstSubtitle: SubtitleData;
    secondSubtitle?: SubtitleData;
    synchronization: number;
    align: Align;
}

export interface Props {
    isProcessing: boolean;
    onSubmit?: (v: FormData) => void;
    firstSubtitleError?: string;
    secondSubtitleError?: string;
}

function SubtitleForm({isProcessing, onSubmit, firstSubtitleError, secondSubtitleError}: Props) {
    const formValuesStore = useLocalStore(createFormValueStore);
    const formStore = useLocalStore(createFormControlStore(formValuesStore));

    // componentWillUnmount
    useEffect(() => {
        return () => formStore.dispose()
    }, [formStore]);

    const submit = () => {
        if (formStore.valid && onSubmit) {
            const subtitle1 = {
                file: formValuesStore.file1 as File,
                fps: formValuesStore.fps1 || undefined,
                encoding: formValuesStore.encoding1 || undefined,
                speed: formValuesStore.speed1 || undefined,
            };
            const subtitle2 = formValuesStore.singleMode ? undefined : {
                file: formValuesStore.file2 as File,
                fps: formValuesStore.fps2 || undefined,
                encoding: formValuesStore.encoding2 || undefined,
                speed: formValuesStore.speed2 || undefined,
            };
            onSubmit({
                firstSubtitle: subtitle1,
                secondSubtitle: subtitle2,
                synchronization: formValuesStore.synchronization,
                align: formValuesStore.align
            });
        }
    };

    const toString = (e: HTMLInputElement) => e.value.trim();

    const toFloat = (e: HTMLInputElement) => {
        const value = toString(e);
        return value ? parseFloat(value) : null;
    };

    const toInt = (def: number) => (e: HTMLInputElement) => {
        const value = toString(e);
        return value ? parseInt(value) : def;
    };

    const toFile = (e: HTMLInputElement) => {
        const value = e.files;
        return (value && value.length === 1) ? value[0] : null;
    };

    const controls = formStore.controls;
    const firstSubtitleControls = controls.firstSubtitle.controls;
    const secondSubtitleControls = controls.secondSubtitle.controls;
    const [isSingleMode, setSingleMode] = [formValuesStore.singleMode, (v: boolean) => controls.singleMode.setValue(v)];
    const [align, setAlign] = [formValuesStore.align, (v: Align) => secondSubtitleControls.align.setValue(v)];
    return <>
        <Form>
            {
                firstSubtitleError &&
                <Alert variant="danger">
                    {firstSubtitleError}
                </Alert>
            }
            <InputControl id="file1" title="First subtitle file" required={true} type="file"
                          control={firstSubtitleControls.file} mapper={toFile}/>
            <InputControl id="fps1" title="FPS (Frames per Second)" type="text"
                          text="Some types of subtitles store frames. Leave it blank if you are not sure. The system asks you if it will be needed."
                          control={firstSubtitleControls.fps} mapper={toFloat}/>
            <InputControl id="encoding1" title="File encoding" type="text" placeholder="auto"
                          text="Leave it blank and the system will try to detect encoding."
                          control={firstSubtitleControls.encoding} mapper={toString}/>
            <InputControl id="speed1" title="Speed" type="text" placeholder="1.0"
                          control={firstSubtitleControls.speed} mapper={toFloat}/>

            <Form.Group controlId="mode">
                <ButtonGroup toggle>
                    <ToggleButton type="radio" variant="primary" value="" checked={isSingleMode}
                                  onChange={() => setSingleMode(true)}>
                        Single mode
                    </ToggleButton>
                    <ToggleButton type="radio" variant="primary" value="" checked={!isSingleMode}
                                  onChange={() => setSingleMode(false)}>
                        Duo mode
                    </ToggleButton>
                </ButtonGroup>
            </Form.Group>

            {formStore.controls.secondSubtitle.active &&
            <>
                {
                    secondSubtitleError &&
                    <Alert variant="danger">
                        {secondSubtitleError}
                    </Alert>
                }
                <InputControl id="file2" title="Second subtitle file" required={true} type="file"
                              text="Lines from this file will be in the top or right screen corner"
                              control={secondSubtitleControls.file} mapper={toFile}/>
                <InputControl id="fps2" title="FPS (Frames per Second)" type="text"
                              text="Some types of subtitles store frames. Leave it blank if you are not sure. The system asks you if it will be needed."
                              control={secondSubtitleControls.fps} mapper={toFloat}/>
                <InputControl id="encoding2" title="File encoding" type="text" placeholder="auto"
                              text="Leave it blank and the system will try to detect encoding."
                              control={secondSubtitleControls.encoding} mapper={toString}/>
                <InputControl id="speed2" title="Speed" type="text" placeholder="1.0"
                              control={secondSubtitleControls.speed} mapper={toFloat}/>

                <InputControl id="synchronization" title="Second subtitle synchronization" type="text" placeholder="0"
                              text="Positive or negative time in ms. This value lets you align start/end time with the first subtitle."
                              control={secondSubtitleControls.synchronization} mapper={toInt(0)}/>

                <Form.Group controlId="align">
                    <Form.Label>Align</Form.Label>
                    <div>
                        <ButtonGroup toggle vertical className="mr-3">
                            <ToggleButton type="radio" variant="primary" value="" checked={align === Align.VERTICAL}
                                          onChange={() => setAlign(Align.VERTICAL)}>
                                {Align.VERTICAL}
                            </ToggleButton>
                            <ToggleButton type="radio" variant="primary" value=""
                                          checked={align === Align.HORIZONTAL_TOP}
                                          onChange={() => setAlign(Align.HORIZONTAL_TOP)}>
                                {Align.HORIZONTAL_TOP}
                            </ToggleButton>
                            <ToggleButton type="radio" variant="primary" value=""
                                          checked={align === Align.HORIZONTAL_BOTTOM}
                                          onChange={() => setAlign(Align.HORIZONTAL_BOTTOM)}>
                                {Align.HORIZONTAL_BOTTOM}
                            </ToggleButton>
                        </ButtonGroup>
                        <AlignSVG align={align} width={110}/>
                    </div>
                </Form.Group>
            </>
            }

            <Form.Group controlId="submit">
                <Button variant="primary" type="button" onClick={submit}
                        size="lg" block disabled={isProcessing || formStore.invalid}>
                    {isProcessing ? "Loading..." : "Process subtitles"}
                </Button>
            </Form.Group>
        </Form>
    </>;
}

export default observer(SubtitleForm);