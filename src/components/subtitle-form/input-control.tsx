import {Form} from 'react-bootstrap';
import React from 'react';
import {UseFormRegisterReturn} from 'react-hook-form/dist/types/form';

export interface UIInputControl<T> {
    id: string;
    title: string;
    text?: string;
    required?: boolean;
    type: 'text' | 'file' | 'number';
    placeholder?: string;
    control: UseFormRegisterReturn;
    error: string | undefined;
}

export default function InputControl<T>(
    {id, title, required, type, placeholder, text, control, error}: UIInputControl<T>
) {
    return (
        <Form.Group controlId={id} className="mb-3">
            <Form.Label>{title}{required ? (<span className="text-danger">*</span>) : null}</Form.Label>
            <Form.Control type={type} placeholder={placeholder} step="0.0000001" isInvalid={!!error} {...control} />
            {
                (error && <Form.Control.Feedback type="invalid">{error}</Form.Control.Feedback>) ||
                (text && <Form.Text className="text-muted">{text}</Form.Text>)
            }
        </Form.Group>);
}