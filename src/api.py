from fastapi import FastAPI, HTTPException, Path
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

from os import environ
from subprocess import run


ZOE = environ.get("ZOE", "./zoe")


STATE_REGEX = r"^([AaBbGgQqSs][A-Xa-x][A-Xa-x]){0,22}[12]$"
ACTION_REGEX = r"^([A-Xa-x]{4})|\+[ABGQSabgqs][A-Xa-x]{2}$"


app = FastAPI()
app.mount("/ui", StaticFiles(directory="ui", html=True))


class EngineResponse(BaseModel):
    log: str


class ActionsResponse(EngineResponse):
    actions: list[str]


class ActResponse(EngineResponse):
    state: str
    actions: list[str]


def zoe(*args) -> (str, str):
    p = run([ZOE] + list(args), capture_output=True, encoding="utf-8")

    if p.returncode:
        raise HTTPException(status_code=422, detail=p.stderr)

    return p.stdout, p.stderr


def get_actions(state: str) -> (list[str], str):
    stdout, stderr = zoe("-l", state)
    return ([line for line in stdout.strip().split("\n")], stderr)


@app.get("/state/{state}/actions", response_model=ActionsResponse)
async def state_actions(state: str = Path(regex=STATE_REGEX)) -> ActionsResponse:
    actions, stderr = get_actions(state)
    return ActionsResponse(actions=actions, log=stderr)


@app.get("/state/{state}/act/{action}", response_model=ActResponse)
async def state_act(
    state: str = Path(regex=STATE_REGEX), action: str = Path(regex=ACTION_REGEX)
) -> ActResponse:
    actions, _ = get_actions(state)
    if action not in actions:
        raise HTTPException(status_code=422, detail="Illegal action")

    stdout, stderr = zoe("-a", action, state)
    state = stdout.strip()

    actions, _ = get_actions(state)

    return ActResponse(state=state, actions=actions, log=stderr)
