import requests

params = {"temp": 5, "humidity": 85}
resp = requests.get("http://127.0.0.1:5000/predict", params=params)
print(resp.json())