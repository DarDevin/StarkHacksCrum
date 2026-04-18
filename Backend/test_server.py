#!/usr/bin/env python3
"""
Test script for the FastAPI backend server.
Run this after starting the server to test the endpoints.
"""

import requests
import time

def test_server():
    base_url = "http://10.10.8.55:8000"

    # Test root endpoint
    print("Testing root endpoint...")
    try:
        response = requests.get(f"{base_url}/")
        print(f"Status: {response.status_code}")
        print(f"Response: {response.json()}")
    except Exception as e:
        print(f"Error: {e}")
        return

    # Test telemetry endpoint with first ping
    print("\nTesting telemetry endpoint (first ping)...")
    data = {
        "latitude": 40.4314,
        "longitude": -86.9215,
        "angle_delta": 0.0,
        "is_salting": False,
        "is_first_ping": True
    }

    try:
        response = requests.post(f"{base_url}/telemetry", json=data)
        print(f"Status: {response.status_code}")
        print(f"Response: {response.json()}")
    except Exception as e:
        print(f"Error: {e}")

    # Test telemetry endpoint with second ping
    print("\nTesting telemetry endpoint (second ping)...")
    data = {
        "latitude": 40.4314,
        "longitude": -86.9192,
        "angle_delta": 15.0,
        "is_salting": False,
        "is_first_ping": False
    }

    try:
        response = requests.post(f"{base_url}/telemetry", json=data)
        print(f"Status: {response.status_code}")
        print(f"Response: {response.json()}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    test_server()