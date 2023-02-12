import Grid from "./Grid.js";
import { e } from "./shortcuts.js";

export default function App() {
  const [state, setState] = React.useState(location.hash.slice(1));

  React.useEffect(() => {
    window.addEventListener("hashchange", () => {
      setState(location.hash.slice(1));
    });
  }, []);

  return e(Grid, { state: state });
}
