from fastapi import FastAPI, Path
from pydantic import BaseModel

from os import environ
from subprocess import run


ZOE = environ.get("ZOE", "./zoe")


app = FastAPI()


class EngineResponse(BaseModel):
    log: str


class ActionsResponse(EngineResponse):
    actions: list[str]


@app.get("/actions/{state}", response_model=ActionsResponse)
async def actions(
    state: str = Path(regex=r"^([AaBbGgQqSs][A-Xa-x][A-Xa-x]){1,22}[12]$"),
) -> ActionsResponse:
    p = run([ZOE, "-l", state], capture_output=True, encoding="utf-8")

    return ActionsResponse(
        actions=[line for line in p.stdout.strip().split("\n")], log=p.stderr
    )
