import Grid from "./Grid.js";
import Hands from "./Hands.js";
import { e } from "./shortcuts.js";
import { axialToString } from "./util.js";

export default function App() {
  const [state, setState] = React.useState(location.hash.slice(1));
  const [actions, setActions] = React.useState([]);
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

    for (const action in actions) {
      if (action == newActionInput) {
        location.hash = actions[action];
        setActionInput("");
        break;
      } else if (action.startsWith(newActionInput)) {
        setActionInput(newActionInput);
        break;
      }
    }
  }

  return e(
    React.Fragment,
    null,
    e(Grid, { state, handleHexClick }),
    e(Hands, { state })
  );
}
