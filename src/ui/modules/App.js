import Grid from "./Grid.js";
import { e } from "./shortcuts.js";

export default function App() {
  const [state, setState] = React.useState(location.hash.slice(1));

  React.useEffect(() => {
    console.log("App effect");

    function handleHashChange() {
      console.log('hash change');
      setState(location.hash.slice(1));
    }
    window.addEventListener("hashchange", handleHashChange);

    return () => {
      window.removeEventListener('hashchange', handleHashChange);
    };
  }, []);

  return e(Grid, { state: state });
}
