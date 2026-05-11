import requests

params = {"temp": 5, "humidity": 85}
resp = requests.get("http://10.10.8.55:5000/predict", params=params)
print(resp.json())