import { Player } from "./enum.js";
import Grid from "./Grid.js";
import Hand from "./Hand.js";
import { e } from "./shortcuts.js";

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

  function handleActionInput(actionInputPart) {
    console.log("input:", actionInputPart);
    const newActionInput = actionInput + actionInputPart;

    for (const action in actions) {
      if (action == newActionInput) {
        console.log("action:", newActionInput);
        console.log("finsihed");
        location.hash = actions[action];
        setActionInput("");
        break;
      } else if (action.startsWith(newActionInput)) {
        setActionInput(newActionInput);
        console.log("action:", newActionInput);
        break;
      }
    }
  }

  return e(
    React.Fragment,
    null,
    e(Grid, { state, handleActionInput }),
    e(Hand, { state, player: Player.P1, handleActionInput }),
    e(Hand, { state, player: Player.P2, handleActionInput }),
  );
}
