# Salt Bot Backend Server

A FastAPI-based backend server for the autonomous salt-spreading robot system.

## Features

- **Telemetry Processing**: Receives GPS coordinates, orientation data, and salting status from the robot
- **Decision Making**: Determines when to start/stop salting based on road segments and intersection logic
- **Database Storage**: Tracks salting status of road segments using SQLite
- **RESTful API**: Clean API endpoints for robot communication

## Setup

1. **Install Dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

2. **Initialize Database**:
   The database is automatically created when the server starts via the lifespan event.

3. **Run the Server**:
   ```bash
   python run_server.py
   ```

   Or directly:
   ```bash
   python main.py
   ```

   The server will start on `http://10.10.8.55:8000`

## API Endpoints

### GET /
Returns server status.

**Response:**
```json
{
  "status": "server running"
}
```

### POST /telemetry
Receives telemetry data from the robot and returns salting decisions.

**Request Body:**
```json
{
  "latitude": 40.4314,
  "longitude": -86.9215,
  "angle_delta": 15.0,
  "is_salting": false,
  "is_first_ping": true
}
```

**Response:**
```json
{
  "status": "ok",
  "input": {
    "latitude": 40.4314,
    "longitude": -86.9215,
    "angle_delta": 15.0,
    "is_salting": false,
    "is_first_ping": true
  },
  "decision": {
    "start_salting": false
  }
}
```

## Testing

Run the test script to verify the server is working:

```bash
python test_server.py
```

## Architecture

- **main.py**: FastAPI application with endpoints
- **run_server.py**: Convenience script to start the server
- **roadlogicanddata.py**: Core logic for intersection detection, segment management, and decision making
- **sqlite_make_db.py**: Database schema (legacy, database is now auto-initialized)
- **test_server.py**: Test script to verify server functionality

## Database

The system uses SQLite with a `saltbot.db` file containing:
- `segments` table: Tracks which road segments have been salted and when

## Decision Logic

The robot sends telemetry pings as it moves. The server:

1. **First Ping**: Records starting position for calibration
2. **Second Ping**: Calculates initial heading based on movement
3. **Subsequent Pings**: Updates heading and checks if robot is at an intersection
4. **Intersection Logic**: Determines if the current road segment needs salting based on:
   - Whether the segment exists in the predefined map
   - Whether the segment has been salted recently (within 8 hours)
   - Robot's current heading matching the segment direction