import React from 'react';
import {Align} from '../types';

function AlignSVG({align, width}: { align: Align, width: number }) {
    let pos1: { x: number, y: number };
    let pos2: { x: number, y: number };
    switch (align) {
        case Align.VERTICAL:
            pos1 = {x: 512 / 2, y: 120};
            pos2 = {x: 512 / 2, y: 420};
            break;
        case Align.HORIZONTAL_TOP:
            pos1 = {x: 512 / 2 / 2, y: 120};
            pos2 = {x: 512 / 2 / 2 * 3, y: 120};
            break;
        case Align.HORIZONTAL_BOTTOM:
            pos1 = {x: 512 / 2 / 2, y: 420};
            pos2 = {x: 512 / 2 / 2 * 3, y: 420};
            break;
    }

    const generateStyle = (x: number, y: number) => {
        return {
            fill: '#BDC3C7',
            transition: 'transform 700ms ease-in-out',
            transform: `translate(${x}px, ${y}px)`
        };
    }

    return <svg version="1.1" xmlns="http://www.w3.org/2000/svg" x="0px" y="0px" viewBox="0 0 512 512"
                style={{maxWidth: width + 'px'}}>
        <rect x="0" y="65" style={{fill: '#ECF0F1'}} width="512" height="512"/>
        <path style={{fill: '#687492'}}
              d="M512,65.931H0V28.69C0,12.844,12.844,0,28.69,0H483.31C499.156,0,512,12.844,512,28.69V65.931z"/>
        <g>
            <rect x="36" y="20" style={{fill: '#E64C3C'}} width="30" height="30"/>
            <rect x="100" y="20" style={{fill: '#F0C419'}} width="30" height="30"/>
            <rect x="165" y="20" style={{fill: '#24AE5F'}} width="30" height="30"/>
        </g>
        <g>
            <rect style={generateStyle(pos1.x - (170 / 2), pos1.y)} width="170" height="20"/>
            <rect style={generateStyle(pos1.x - (230 / 2), pos1.y + 40)} width="230" height="20"/>
        </g>
        <g>
            <rect style={generateStyle(pos2.x - (170 / 2), pos2.y)} width="170" height="20"/>
            <rect style={generateStyle(pos2.x - (230 / 2), pos2.y + 40)} width="230" height="20"/>
        </g>
    </svg>
}

export default AlignSVG;