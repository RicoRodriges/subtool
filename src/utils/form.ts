import {
    FormControl,
    InputFormControl,
    ValidationEvent,
    ValidationEventTypes
} from "@quantumart/mobx-form-validation-kit";
import React from "react";

// Override value=event.target.value logic to custom
export class InputCustomFormControl {
    static bindActions<T>(formControl: FormControl<T>, mapper: (e: HTMLInputElement) => T) {
        return InputFormControl.bindActions(formControl as any, {
            onChange: (e: React.ChangeEvent<HTMLInputElement>) => {
                formControl.value = mapper(e.target);
            }
        });
    }
}

export const notNaN = <TEntity extends (number | null)>(
    message: string,
    eventType = ValidationEventTypes.Error,
) => {
    return async (control: FormControl<TEntity>): Promise<ValidationEvent[]> => {
        const value = control.value;
        if (value != null && isNaN(value as number)) {
            return [
                {
                    message,
                    key: 'notNaN',
                    type: eventType,
                },
            ];
        }
        return [];
    };
};