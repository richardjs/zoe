import App from "./modules/App.js";
import { e } from "./modules/shortcuts.js";

//let container = document.getElementById('container');
//let canvas = document.getElementById('grid');
//let ctx = canvas.getContext('2d');
//
//function render() {
//    canvas.width = canvas.parentElement.clientWidth;
//    canvas.height = canvas.parentElement.clientHeight;
//
//    let grid = new Grid(location.hash.slice(1));
//    let gridCanvas = grid.render(ctx);
//
//    let aspectRatio = gridCanvas.width / gridCanvas.height;
//    let drawWidth = Math.min(canvas.width, gridCanvas.width);
//    let drawHeight = Math.min(canvas.height, gridCanvas.height);
//
//    ctx.drawImage(
//        gridCanvas,
//        canvas.width/2 - drawWidth/2,
//        canvas.height/2 - drawHeight/2,
//        drawWidth,
//        drawHeight
//    );
//}
//
//render();
//window.addEventListener('resize', render);
//window.addEventListener('hashchange', render);

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(
  e(React.StrictMode, null, e(App, { state: location.hash.slice(1) }))
);
