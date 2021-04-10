import {Form} from "react-bootstrap";
import React from "react";
import {FormControl} from "@quantumart/mobx-form-validation-kit";
import {InputCustomFormControl} from "../../utils/form";
import {observer} from "mobx-react";

export interface UIInputControl<T> {
    id: string;
    title: string;
    text?: string;
    required?: boolean;
    type: string;
    placeholder?: string;
    control: FormControl<T>;
    mapper: (e: HTMLInputElement) => T;
}

function InputControl<T>({id, title, required, type, placeholder, text, control, mapper}: UIInputControl<T>) {
    const isInvalid = control.errors && control.errors.length > 0 && control.touched;
    return (
        <Form.Group controlId={id}>
            <Form.Label>{title}{required ? (<span className="text-danger">*</span>) : null}</Form.Label>
            <Form.Control type={type} placeholder={placeholder}
                          isInvalid={isInvalid} {...InputCustomFormControl.bindActions(control, mapper)}/>
            {
                (isInvalid &&
                    <Form.Control.Feedback type="invalid">{control.errors[0].message}</Form.Control.Feedback>) ||
                (text && <Form.Text className="text-muted">{text}</Form.Text>)
            }
        </Form.Group>);
}

export default observer(InputControl);