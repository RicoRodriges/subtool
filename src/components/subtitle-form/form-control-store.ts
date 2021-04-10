import {IFormValueStore} from "./form-value-store";
import {
    AbstractControls,
    FormControl,
    FormGroup,
    minValue,
    required,
    wrapperActivateValidation
} from "@quantumart/mobx-form-validation-kit";
import {notNaN} from "../../utils/form";
import {Align} from "../../types";

export interface IMainSubtitleControls extends AbstractControls {
    file: FormControl<File | null>;
    fps: FormControl<number | null>;
    encoding: FormControl<string>;
    speed: FormControl<number | null>;
}

export interface ISecondSubtitleControls extends IMainSubtitleControls {
    synchronization: FormControl<number>;
    align: FormControl<Align>;
}


export interface IFormControls extends AbstractControls {
    firstSubtitle: FormGroup<IMainSubtitleControls>;
    singleMode: FormControl<boolean>;
    secondSubtitle: FormGroup<ISecondSubtitleControls>;
}

export function createFormControlStore(formValues: IFormValueStore) {
    return () => new FormGroup<IFormControls>({
        firstSubtitle: new FormGroup<IMainSubtitleControls>({
            file: new FormControl<File | null>(
                formValues.file1,
                [required('Please choose the file')],
                v => formValues.file1 = v
            ),
            fps: new FormControl<number | null>(
                formValues.fps1,
                [notNaN('Value must be a float value'), wrapperActivateValidation(
                    c => c.value != null,
                    // TODO: 1 as any??? WTF
                    [minValue(1 as any, 'This value must be a positive float value')]
                )],
                v => formValues.fps1 = v
            ),
            encoding: new FormControl<string>(
                formValues.encoding1,
                [],
                v => formValues.encoding1 = v
            ),
            speed: new FormControl<number | null>(
                formValues.speed1,
                [notNaN('Value must be a positive float value'), wrapperActivateValidation(
                    c => c.value != null,
                    // TODO
                    [minValue(Number.EPSILON as any, 'This value must be greater than zero')]
                )],
                v => formValues.speed1 = v
            ),
        }),

        singleMode: new FormControl<boolean>(
            formValues.singleMode,
            [],
            v => formValues.singleMode = v
        ),

        secondSubtitle: new FormGroup<ISecondSubtitleControls>({
            file: new FormControl<File | null>(
                formValues.file2,
                [required('Please choose the file')],
                v => formValues.file2 = v
            ),
            fps: new FormControl<number | null>(
                formValues.fps2,
                [notNaN('Value must be a float value'), wrapperActivateValidation(
                    c => c.value != null,
                    // TODO: 1 as any??? WTF
                    [minValue(1 as any, 'This value must be a positive float value')]
                )],
                v => formValues.fps2 = v
            ),
            encoding: new FormControl<string>(
                formValues.encoding2,
                [],
                v => formValues.encoding2 = v
            ),
            speed: new FormControl<number | null>(
                formValues.speed2,
                [notNaN('Value must be a positive float value'), wrapperActivateValidation(
                    c => c.value != null,
                    // TODO
                    [minValue(Number.EPSILON as any, 'This value must be greater than zero')]
                )],
                v => formValues.speed2 = v
            ),
            synchronization: new FormControl<number>(
                formValues.synchronization,
                [notNaN('Value must be an integer value')],
                v => formValues.synchronization = v
            ),
            align: new FormControl<Align>(
                formValues.align,
                [],
                v => formValues.align = v
            ),
        }, [], () => !formValues.singleMode),
    });
}