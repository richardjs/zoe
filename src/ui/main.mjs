import { Grid } from './modules/grid.mjs';


let container = document.getElementById('container');
let canvas = document.getElementById('grid');
let ctx = canvas.getContext('2d');

function render() {
    canvas.height = canvas.parentElement.clientHeight;
    canvas.width = canvas.parentElement.clientWidth;

    let grid = new Grid(location.hash.slice(1));
    let gridCanvas = grid.render(ctx);

    ctx.drawImage(
        gridCanvas,
        canvas.width/2 - gridCanvas.width/2,
        canvas.height/2 - gridCanvas.height/2
    );
}

render();
window.addEventListener('resize', render);
window.addEventListener('hashchange', render);
