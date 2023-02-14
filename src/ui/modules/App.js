import Grid from "./Grid.js";
import { e } from "./shortcuts.js";
import { axialToString } from "./util.js";

export default function App() {
  const [state, setState] = React.useState(location.hash.slice(1));
  const [actions, setActions] = React.useState(null);
  const [actionInput, setActionInput] = React.useState("");

  React.useEffect(() => {
    function handleHashChange() {
      setState(location.hash.slice(1));
    }
    window.addEventListener("hashchange", handleHashChange);

    return () => {
      window.removeEventListener("hashchange", handleHashChange);
    };
  }, []);

  React.useEffect(() => {
    let ignore = false;

    function fetchActions() {
      fetch("/state/" + state + "/actions")
        .then((response) => response.json())
        .then((json) => {
          if (ignore) return;
          setActions(json.actions);
        });
    }
    fetchActions();

    return () => {
      ignore = true;
    };
  }, [state]);

  function handleHexClick({ q, r }) {
    const hexString = axialToString({ q, r });
    const newActionInput = actionInput + hexString;
    for (let action of actions) {
      if (action == newActionInput) {
        console.log("action entered:", action);
        setActionInput("");
        break;
      } else if (action.startsWith(newActionInput)) {
        console.log("building action:", newActionInput);
        setActionInput(newActionInput);
        break;
      }
    }
  }

  return e(Grid, { state, handleHexClick });
}
