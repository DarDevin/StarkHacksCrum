from flask import Flask, request, jsonify
from roadlogicanddata import make_decision, init_db, SEGMENTS

app = Flask(__name__)

# ── GPS ENDPOINT ──────────────────────────────────────────────────
# ESP32 posts here with its current state, gets back salt/don't salt
@app.route("/gps", methods=["POST"])
def receive_gps():
    data = request.get_json()
    result = make_decision(
        data["lat"],
        data["lng"],
        data["angle_delta"],
        data["is_salting"],
        data["is_first_ping"]
    )
    return jsonify(result)

# ── STATUS ENDPOINT ───────────────────────────────────────────────
# Map app polls this to color segments red/gray
@app.route("/status", methods=["GET"])
def get_status():
    return jsonify({
        seg_id: {"is_salted": seg.is_salted()}
        for seg_id, seg in SEGMENTS.items()
    })

# ── RUN ───────────────────────────────────────────────────────────
if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=8000, debug=True)
