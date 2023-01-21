import { Grid } from './modules/grid.mjs';


let container = document.getElementById('container');
let canvas = document.getElementById('grid');
let ctx = canvas.getContext('2d');

canvas.height = canvas.parentElement.clientHeight;
canvas.width = canvas.parentElement.clientWidth;

let grid = new Grid();
grid.render(ctx)
