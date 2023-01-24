import { Player, Type } from './enum.mjs';
import { HEX_SIZE } from './hex.mjs';
import { Piece } from './piece.mjs';


const MAX_PIECES = 22;


export class Grid {
    constructor(state) {
        this.grid = [];

        this.minX = MAX_PIECES;
        this.maxX = 0;
        this.minY = MAX_PIECES;
        this.maxY = 0;

        for (let i = 0; i + 2 < state.length; i += 3) {
            let piece = Piece.fromChar(state[i]);
            let q = state[i+1].charCodeAt(0) - 'a'.charCodeAt(0);
            let r = state[i+2].charCodeAt(0) - 'a'.charCodeAt(0);

            let x = q;
            let y = 2*r + q;

            if (this.grid[x] === undefined) {
                this.grid[x] = [];
            }

            if (!this.grid[x][y]) {
                this.grid[x][y] = piece;
            } else {
                let p = this.grid[x][y];
                while (p.on_top) {
                    p = p.on_top
                }
                p.on_top = piece;
            }

            this.minX = Math.min(this.minX, x);
            this.maxX = Math.max(this.maxX, x);
            this.minY = Math.min(this.minY, y);
            this.maxY = Math.max(this.maxY, y);
        }
    }

    render() {
        let gridWidth = this.maxX - this.minX + 1;
        let gridHeight = this.maxY - this.minY + 1;

        let canvas = document.createElement('canvas');

        canvas.width = 2*HEX_SIZE + 1.5*HEX_SIZE*(gridWidth - 1);
        canvas.height = HEX_SIZE*Math.sqrt(3) + .5*HEX_SIZE*Math.sqrt(3)*(gridHeight - 1);

        let ctx = canvas.getContext('2d');

        for (let x = 0; x < MAX_PIECES; x+=2) {
            for (let y = 0; y < MAX_PIECES; y += 2) {
                ctx.save();

                ctx.translate(
                    HEX_SIZE + (x - this.minX)*1.5*HEX_SIZE,
                    .5*HEX_SIZE*Math.sqrt(3) + (y - this.minY)*.5*HEX_SIZE*Math.sqrt(3)
                );

                if (this.grid[x] && this.grid[x][y] ) {
                    let piece = this.grid[x][y];
                    piece.render(ctx);
                }

                ctx.translate(
                    1.5*HEX_SIZE,
                    .5*HEX_SIZE*Math.sqrt(3)
                );

                if (this.grid[x+1] && this.grid[x+1][y+1]) {
                    let piece = this.grid[x+1][y+1];
                    piece.render(ctx);
                }

                ctx.restore();
            }
        }

        return canvas;
    }
}
