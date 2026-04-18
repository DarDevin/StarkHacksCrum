from contextlib import asynccontextmanager

from fastapi import FastAPI
from pydantic import BaseModel, Field



# Change this import to match your filename/module name
# Example: if the file is saved as salt_logic.py, use:
from roadlogicanddata import make_decision, init_db
import requests
import uvicorn


@asynccontextmanager
async def lifespan(app: FastAPI):
    init_db()
    yield


app = FastAPI(lifespan=lifespan)


class TelemetryPing(BaseModel):
    latitude: float = Field(..., examples=[40.4252])
    longitude: float = Field(..., examples=[-86.9191])
    angle_delta: float = Field(..., examples=[15.0])
    is_salting: bool = Field(..., examples=[False])
    is_first_ping: bool = Field(..., examples=[True])


@app.get("/")
def root():
    return {"status": "server running"}


@app.post("/telemetry")
def receive_telemetry(data: TelemetryPing):
    decision = make_decision(
        lat=data.latitude,
        lng=data.longitude,
        angle_delta=data.angle_delta,
        is_salting=data.is_salting,
        is_first_ping=data.is_first_ping,
    )

    return {
        "status": "ok",
        "input": {
            "latitude": data.latitude,
            "longitude": data.longitude,
            "angle_delta": data.angle_delta,
            "is_salting": data.is_salting,
            "is_first_ping": data.is_first_ping,
        },
        "decision": decision,
    }

def main():
    uvicorn.run(app, host="0.0.0.0", port=8000)

if __name__ == "__main__":
    main()