# train_and_serve.py
import requests
import pandas as pd
import numpy as np
from datetime import date, timedelta
from xgboost import XGBClassifier
from flask import Flask, request, jsonify
import joblib

# --- 1. Fetch historical weather data from Open-Meteo Archive API ---
def fetch_weather_data(days_back: int = 1000):
    end_date   = date.today() - timedelta(days=1)   # yesterday (archive lags 1 day)
    start_date = end_date - timedelta(days=days_back)

    url = "https://archive-api.open-meteo.com/v1/archive"
    params = {
        "latitude":         40.4259,    # Purdue University
        "longitude":       -86.9081,
        "start_date":       start_date.isoformat(),
        "end_date":         end_date.isoformat(),
        "hourly":           ["temperature_2m", "relative_humidity_2m", "snowfall"],
        "temperature_unit": "celsius",
        "timezone":         "America/Indiana/Indianapolis",
    }

    r = requests.get(url, params=params, timeout=30)
    r.raise_for_status()

    data = r.json()["hourly"]
    df = pd.DataFrame(data)

    # Drop any rows the API returned as null
    df.dropna(subset=["temperature_2m", "relative_humidity_2m", "snowfall"], inplace=True)

    df["is_snowing"] = (df["snowfall"] > 0).astype(int)

    print(f"Fetched {len(df)} hourly records ({start_date} → {end_date})")
    print(f"  Snow hours : {df['is_snowing'].sum()}")
    print(f"  Clear hours: {(df['is_snowing'] == 0).sum()}")

    return df

# --- 2. Train XGBoost ---
def train_model(df: pd.DataFrame):
    X = df[["temperature_2m", "relative_humidity_2m"]]
    y = df["is_snowing"]

    # Class imbalance: snow hours are rare, so tell XGBoost to up-weight them
    snow_count  = y.sum()
    clear_count = (y == 0).sum()
    scale_pos_weight = clear_count / max(snow_count, 1)
    print(f"scale_pos_weight = {scale_pos_weight:.2f}")

    model = XGBClassifier(
        n_estimators=200,
        max_depth=4,
        learning_rate=0.05,
        scale_pos_weight=scale_pos_weight,   # handles class imbalance
        eval_metric="logloss",
        random_state=42,
    )
    model.fit(X, y)
    joblib.dump(model, "snow_model.pkl")
    print("Model trained and saved to snow_model.pkl")
    return model

# --- 3. Flask API ---
app = Flask(__name__)
model = None

@app.route("/predict", methods=["GET"])
def predict():
    """
    Query params:
      temp     – temperature in Celsius
      humidity – relative humidity 0–100
    Example:
      GET /predict?temp=-3&humidity=85
    """
    try:
        temp     = float(request.args.get("temp"))
        humidity = float(request.args.get("humidity"))

        if not (-90 <= temp <= 60):
            return jsonify({"error": "temp out of plausible range (-90 to 60 °C)"}), 400
        if not (0 <= humidity <= 100):
            return jsonify({"error": "humidity must be 0–100"}), 400

        prob = model.predict_proba([[temp, humidity]])[0][1]
        return jsonify({
            "snow_probability": round(float(prob), 4),
            "inputs": {"temp_c": temp, "humidity_pct": humidity},
        })
    except (TypeError, ValueError):
        return jsonify({"error": "Both 'temp' and 'humidity' query params are required"}), 400
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/health", methods=["GET"])
def health():
    return jsonify({"status": "ok", "model_loaded": model is not None})

if __name__ == "__main__":
    df    = fetch_weather_data(days_back=1000)
    model = train_model(df)
    app.run(host="0.0.0.0", port=5000, debug=False)