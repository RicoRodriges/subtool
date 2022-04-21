import {ToTimeString} from './time';

describe('test time utils', () => {
    test('ToTimeString', () => {
        expect(ToTimeString(1_234, '.')).toEqual('00:00:01.234');
        expect(ToTimeString(1_200, '.')).toEqual('00:00:01.200');
        expect(ToTimeString(1_200, ',')).toEqual('00:00:01,200');
        expect(ToTimeString(65_200, ',')).toEqual('00:01:05,200');
        expect(ToTimeString(3_665_200, ',')).toEqual('01:01:05,200');
        expect(ToTimeString(3_665_200, ',', 2)).toEqual('01:01:05,20');
        expect(ToTimeString(3_665_239, ',', 2)).toEqual('01:01:05,24');
    });
});