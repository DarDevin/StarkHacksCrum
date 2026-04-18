#!/usr/bin/env python3
"""
Run script for the Salt Bot Backend Server.
This script starts the FastAPI server on 10.10.8.55:8000
"""

import uvicorn
from main import app

if __name__ == "__main__":
    print("Starting Salt Bot Backend Server...")
    print("Server will be available at: http://10.10.8.55:8000")
    print("Press Ctrl+C to stop the server")
    uvicorn.run(app, host="10.10.8.55", port=8000)