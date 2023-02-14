import App from "./modules/App.js";
import { e } from "./modules/shortcuts.js";

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(e(React.StrictMode, null, e(App)));
