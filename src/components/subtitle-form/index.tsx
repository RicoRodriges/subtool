import React, {useState} from 'react';
import {Align} from '../../types';
import {Alert, Button, Form, ToggleButton, ToggleButtonGroup} from 'react-bootstrap';
import AlignSVG from '../align-svg';
import InputControl from './input-control';
import {useForm} from 'react-hook-form';

export interface SubtitleData {
    file: File;
    fps?: number;
    encoding?: string;
    speed?: number;
}

export interface FormData {
    firstSubtitle: SubtitleData;
    secondSubtitle?: SubtitleData;
    offset?: number;
    align: Align;
}

export interface Props {
    isProcessing: boolean;
    onSubmit: (v: FormData) => void;
    firstSubtitleValid: boolean;
    secondSubtitleValid: boolean;
}

export default function SubtitleForm({isProcessing, onSubmit, firstSubtitleValid, secondSubtitleValid}: Props) {

    const [isSingleMode, setSingleMode] = useState(true);
    const [align, setAlign] = useState(Align.VERTICAL);
    const {register, handleSubmit, formState: {errors}} = useForm();

    const toFloat = (v: string | undefined) => v ? parseFloat(v) : undefined;
    const toInt = (v: string | undefined) => v ? parseInt(v) : undefined;

    const buildAndSubmitFormData = (data: { [p: string]: any }) => {
        const subtitle1: SubtitleData = {
            file: (data.file1 as FileList)[0],
            fps: toFloat(data.fps1),
            encoding: data.encoding1,
            speed: toFloat(data.speed1),
        };
        const subtitle2: SubtitleData | undefined = isSingleMode ? undefined : {
            file: (data.file2 as FileList)[0],
            fps: toFloat(data.fps2),
            encoding: data.encoding2,
            speed: toFloat(data.speed2),
        };
        const offset = toInt(data.offset);
        onSubmit({
            firstSubtitle: subtitle1,
            secondSubtitle: subtitle2,
            offset: offset,
            align: align,
        });
    };

    return <>
        <Form onSubmit={handleSubmit(buildAndSubmitFormData)}>
            {
                !firstSubtitleValid && <Alert variant="danger">File format was not recognized</Alert>
            }
            <InputControl id="file1" title="First subtitle file" required={true} type="file"
                          control={register('file1', {required: true})}
                          error={errors.file1 ? "File is required" : undefined}/>
            <InputControl id="fps1" title="FPS (Frames per Second)" type="number"
                          text="Some types of subtitles store frames. Leave it blank if you are not sure."
                          control={register('fps1', {min: 1})}
                          error={errors.fps1 ? "FPS must be >= 1" : undefined}/>
            <InputControl id="encoding1" title="File encoding" type="text" placeholder="auto"
                          text="Leave it blank and the system will try to detect encoding."
                          control={register('encoding1')}
                          error={undefined}/>
            <InputControl id="speed1" title="Speed" type="number" placeholder="1.0"
                          control={register('speed1', {min: 0.0000001})}
                          error={errors.speed1 ? "Speed must be > 0" : undefined}/>

            <Form.Group controlId="mode" className="mb-3">
                <ToggleButtonGroup type="radio" name="mode" value={isSingleMode ? "s" : "d"}>
                    <ToggleButton id="single" type="radio" variant="primary" value="s"
                                  onChange={() => setSingleMode(true)}>
                        Single mode
                    </ToggleButton>
                    <ToggleButton id="duo" type="radio" variant="primary" value="d"
                                  onChange={() => setSingleMode(false)}>
                        Duo mode
                    </ToggleButton>
                </ToggleButtonGroup>
            </Form.Group>

            {!isSingleMode &&
            <>
                {!secondSubtitleValid && <Alert variant="danger">File format was not recognized</Alert>}
                <InputControl id="file2" title="Second subtitle file" required={true} type="file"
                              text="Lines from this file will be in the top or right screen corner"
                              control={register('file2', {required: true})}
                              error={errors.file2 ? "File is required" : undefined}/>
                <InputControl id="fps2" title="FPS (Frames per Second)" type="number"
                              text="Some types of subtitles store frames. Leave it blank if you are not sure."
                              control={register('fps2', {min: 1})}
                              error={errors.fps2 ? "FPS must be >= 1" : undefined}/>
                <InputControl id="encoding2" title="File encoding" type="text" placeholder="auto"
                              text="Leave it blank and the system will try to detect encoding."
                              control={register('encoding2')}
                              error={undefined}/>
                <InputControl id="speed2" title="Speed" type="number" placeholder="1.0"
                              control={register('speed2', {min: 0.0000001})}
                              error={errors.speed2 ? "Speed must be > 0" : undefined}/>

                <InputControl id="offset" title="Second subtitle offset" type="number" placeholder="0"
                              text="Positive or negative time in ms. This value lets you align start/end time with the first subtitle file."
                              control={register('offset')}
                              error={undefined}/>

                <Form.Group controlId="align" className="mb-3">
                    <Form.Label>Align</Form.Label>
                    <div>
                        <ToggleButtonGroup vertical className="me-3" name="align" value={align}>
                            <ToggleButton id="vertical" type="radio" variant="primary" value={Align.VERTICAL}
                                          onChange={() => setAlign(Align.VERTICAL)}>
                                {Align.VERTICAL}
                            </ToggleButton>
                            <ToggleButton id="top" type="radio" variant="primary" value={Align.HORIZONTAL_TOP}
                                          onChange={() => setAlign(Align.HORIZONTAL_TOP)}>
                                {Align.HORIZONTAL_TOP}
                            </ToggleButton>
                            <ToggleButton id="bottom" type="radio" variant="primary" value={Align.HORIZONTAL_BOTTOM}
                                          onChange={() => setAlign(Align.HORIZONTAL_BOTTOM)}>
                                {Align.HORIZONTAL_BOTTOM}
                            </ToggleButton>
                        </ToggleButtonGroup>
                        <AlignSVG align={align} width={110}/>
                    </div>
                </Form.Group>
            </>
            }

            <Form.Group controlId="submit">
                <Button variant="primary" type="submit"
                        size="lg" disabled={isProcessing}>
                    {isProcessing ? "Loading..." : "Process subtitles"}
                </Button>
            </Form.Group>
        </Form>
    </>;
}