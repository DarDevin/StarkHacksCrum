# train_and_serve.py
import requests
import pandas as pd
import numpy as np
from xgboost import XGBClassifier
from flask import Flask, request, jsonify
import joblib

# --- 1. Fetch weather data from API ---
# Example using Open-Meteo (free, no API key needed)
def fetch_weather_data():
    url = "https://api.open-meteo.com/v1/forecast"
    params = {
        "latitude": 40.4259,   # Purdue University
        "longitude": -86.9081,
        "hourly": ["temperature_2m", "relative_humidity_2m", "snowfall"],
        "past_days": 92,
        "temperature_unit": "celsius"
    }
    r = requests.get(url, params=params)
    data = r.json()["hourly"]
    df = pd.DataFrame(data)
    df["is_snowing"] = (df["snowfall"] > 0).astype(int)
    return df

# --- 2. Train XGBoost ---
def train_model(df):
    X = df[["temperature_2m", "relative_humidity_2m"]]
    y = df["is_snowing"]
    model = XGBClassifier(n_estimators=100, max_depth=4, use_label_encoder=False, eval_metric="logloss")
    model.fit(X, y)
    joblib.dump(model, "snow_model.pkl")
    print("Model trained and saved.")
    return model

# --- 3. Flask API ---
app = Flask(__name__)
model = None

@app.route("/predict", methods=["GET"])
def predict():
    try:
        temp = float(request.args.get("temp"))        # Celsius
        humidity = float(request.args.get("humidity")) # 0-100
        prob = model.predict_proba([[temp, humidity]])[0][1]
        return jsonify({"snow_probability": round(float(prob), 4)})
    except Exception as e:
        return jsonify({"error": str(e)}), 400

if __name__ == "__main__":
    df = fetch_weather_data()
    model = train_model(df)
    app.run(host="0.0.0.0", port=5000)