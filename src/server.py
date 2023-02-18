from os import environ
from subprocess import run

from fastapi import FastAPI, HTTPException, Path
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel


ZOE = environ.get("ZOE", "./zoe")


STATE_REGEX = r"^([AaBbGgQqSs][A-Xa-x][A-Xa-x]){0,22}[12]$"


app = FastAPI()


class EngineResponse(BaseModel):
    log: str


class ActionsResponse(EngineResponse):
    actions: dict[str, str]


def zoe(*args) -> (str, str):
    p = run([ZOE] + list(args), capture_output=True, encoding="utf-8")

    if p.returncode:
        raise HTTPException(status_code=422, detail=p.stderr)

    return p.stdout, p.stderr


def get_actions(state: str) -> (list[str], str):
    stdout, stderr = zoe("-l", state)
    return (stdout.strip().split("\n"), stderr)


@app.get("/state/{state}/actions", response_model=ActionsResponse)
async def state_actions(state: str = Path(regex=STATE_REGEX)) -> ActionsResponse:
    actions, stderr = get_actions(state)
    action_states = {action: zoe("-a", action, state)[0].strip() for action in actions}
    return ActionsResponse(actions=action_states, log=stderr)


app.mount("/", StaticFiles(directory="ui", html=True))
