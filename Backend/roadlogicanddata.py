from datetime import datetime, timedelta
import sqlite3
import math

DB_FILE = "saltbot.db"
SALT_DURATION_HOURS = 8
INTERSECTION_RADIUS = 0.0002
MAX_ANGLE_MISMATCH  = 30       # degrees — heading must be within this of an exit
MIN_MOVEMENT_DEG    = 0.00001  # ~1 meter — must move at least this much to calibrate

# ── ROBOT STATE ───────────────────────────────────────────────────
robot_state = {
    "current_angle": None,
    "start_lat":     None,
    "start_lng":     None,
    "calibrated":    False,
}

# ── SEGMENT CLASS ─────────────────────────────────────────────────
class Segment:
    def __init__(self, segment_id: str):
        self.segment_id = segment_id

    def is_salted(self) -> bool:
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        c.execute("SELECT salted_at FROM segments WHERE segment_id = ?", (self.segment_id,))
        row = c.fetchone()
        conn.close()

        if row is None or row[0] is None:
            return False

        salted_time = datetime.fromisoformat(row[0])
        return datetime.utcnow() < salted_time + timedelta(hours=SALT_DURATION_HOURS)

    def mark_salted(self):
        now = datetime.utcnow().isoformat()
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        c.execute("""
            INSERT INTO segments (segment_id, salted_at)
            VALUES (?, ?)
            ON CONFLICT(segment_id) DO UPDATE SET salted_at = ?
        """, (self.segment_id, now, now))
        conn.commit()
        conn.close()

    def mark_unsalted(self):
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        c.execute("UPDATE segments SET salted_at = NULL WHERE segment_id = ?", (self.segment_id,))
        conn.commit()
        conn.close()

# ── ALL SEGMENTS — must be defined before Exit ────────────────────
SEGMENTS = {
    # Stadium (E-W)
    "Stadium_Martin_to_Russell":     Segment("Stadium_Martin_to_Russell"),
    "Stadium_Russell_to_University": Segment("Stadium_Russell_to_University"),

    # 3rd (E-W)
    "3rd_Martin_to_Russell":         Segment("3rd_Martin_to_Russell"),
    "3rd_Russell_to_University":     Segment("3rd_Russell_to_University"),

    # 1st (E-W)
    "1st_MacArthur_to_Martin":       Segment("1st_MacArthur_to_Martin"),
    "1st_Martin_to_Russell":         Segment("1st_Martin_to_Russell"),
    "1st_Russell_to_University":     Segment("1st_Russell_to_University"),

    # State (E-W)
    "State_MacArthur_to_Martin":     Segment("State_MacArthur_to_Martin"),
    "State_Martin_to_Russell":       Segment("State_Martin_to_Russell"),
    "State_Russell_to_University":   Segment("State_Russell_to_University"),

    # University (N-S)
    "University_Stadium_to_3rd":     Segment("University_Stadium_to_3rd"),
    "University_3rd_to_1st":         Segment("University_3rd_to_1st"),
    "University_1st_to_State":       Segment("University_1st_to_State"),

    # Russell (N-S)
    "Russell_Stadium_to_3rd":        Segment("Russell_Stadium_to_3rd"),
    "Russell_3rd_to_1st":            Segment("Russell_3rd_to_1st"),
    "Russell_1st_to_State":          Segment("Russell_1st_to_State"),

    # Martin (N-S)
    "Martin_Stadium_to_3rd":         Segment("Martin_Stadium_to_3rd"),
    "Martin_3rd_to_1st":             Segment("Martin_3rd_to_1st"),
    "Martin_1st_to_State":           Segment("Martin_1st_to_State"),

    # MacArthur (N-S) — only State to 1st
    "MacArthur_State_to_1st":        Segment("MacArthur_State_to_1st"),
}

# ── EXIT CLASS ────────────────────────────────────────────────────
class Exit:
    def __init__(self, segment_id: str, angle: float):
        self.segment = SEGMENTS.get(segment_id)
        self.angle   = angle

# ── INTERSECTION CLASS ────────────────────────────────────────────
class Intersection:
    def __init__(self, id: str, lat: float, lng: float, exits: list):
        self.id    = id
        self.lat   = lat
        self.lng   = lng
        self.exits = exits

# ── INTERSECTIONS ─────────────────────────────────────────────────
INTERSECTIONS = [
    Intersection("Stadium_Martin",     40.43144861233744,  -86.92155685324049, exits=[
        Exit("Stadium_Martin_to_Russell",      90),
        Exit("Martin_Stadium_to_3rd",         180),
    ]),
    Intersection("Stadium_Russell",    40.431403975101404, -86.91918862028561, exits=[
        Exit("Stadium_Martin_to_Russell",     270),
        Exit("Stadium_Russell_to_University",  90),
        Exit("Russell_Stadium_to_3rd",        180),
    ]),
    Intersection("Stadium_University", 40.4313765060136,   -86.91668054898032, exits=[
        Exit("Stadium_Russell_to_University", 270),
        Exit("University_Stadium_to_3rd",     180),
    ]),
    Intersection("3rd_University",     40.42725066143649,  -86.91665049600748, exits=[
        Exit("University_Stadium_to_3rd",       0),
        Exit("University_3rd_to_1st",          180),
        Exit("3rd_Russell_to_University",      270),
    ]),
    Intersection("3rd_Russell",        40.42728341678711,  -86.91913230715227, exits=[
        Exit("3rd_Russell_to_University",      90),
        Exit("3rd_Martin_to_Russell",         270),
        Exit("Russell_Stadium_to_3rd",          0),
        Exit("Russell_3rd_to_1st",            180),
    ]),
    Intersection("3rd_Martin",         40.427342659107545, -86.9220088054924,  exits=[
        Exit("3rd_Martin_to_Russell",          90),
        Exit("Martin_Stadium_to_3rd",          15),
        Exit("Martin_3rd_to_1st",             195),
    ]),
    Intersection("1st_University",     40.42517265448467,  -86.91668657560525, exits=[
        Exit("University_3rd_to_1st",           0),
        Exit("University_1st_to_State",        180),
        Exit("1st_Russell_to_University",      270),
    ]),
    Intersection("1st_Russell",        40.425218154352194, -86.91913813722454, exits=[
        Exit("1st_Russell_to_University",      90),
        Exit("1st_Martin_to_Russell",         270),
        Exit("Russell_3rd_to_1st",              0),
        Exit("Russell_1st_to_State",          180),
    ]),
    Intersection("1st_Martin",         40.42529370123632,  -86.92202512587554, exits=[
        Exit("1st_Martin_to_Russell",          90),
        Exit("1st_MacArthur_to_Martin",       270),
        Exit("Martin_3rd_to_1st",             345),
        Exit("Martin_1st_to_State",           195),
    ]),
    Intersection("1st_MacArthur",      40.425322023069434, -86.92576876532257, exits=[
        Exit("1st_MacArthur_to_Martin",        90),
        Exit("MacArthur_State_to_1st",        180),
    ]),
    Intersection("State_University",   40.424132151555426, -86.91665764187223, exits=[
        Exit("University_1st_to_State",         0),
        Exit("State_Russell_to_University",   270),
    ]),
    Intersection("State_Russell",      40.4242019233523,   -86.91910334463955, exits=[
        Exit("State_Russell_to_University",    90),
        Exit("State_Martin_to_Russell",       270),
        Exit("Russell_1st_to_State",            0),
    ]),
    Intersection("State_Martin",       40.42425515055251,  -86.92172219778557, exits=[
        Exit("State_Martin_to_Russell",        90),
        Exit("State_MacArthur_to_Martin",     270),
        Exit("Martin_1st_to_State",             0),
    ]),
    Intersection("State_MacArthur",    40.42427790340882,  -86.92575600650737, exits=[
        Exit("State_MacArthur_to_Martin",      90),
        Exit("MacArthur_State_to_1st",          0),
    ]),
]

# ── ANGLE HELPERS ─────────────────────────────────────────────────
def calculate_bearing(lat1: float, lng1: float, lat2: float, lng2: float) -> float:
    """
    Calculate compass bearing (degrees clockwise from north) of the vector
    from point 1 to point 2.

    0°   = moved north
    90°  = moved east
    180° = moved south
    270° = moved west
    """
    lat1_rad = math.radians(lat1)
    lat2_rad = math.radians(lat2)
    d_lng    = math.radians(lng2 - lng1)

    x = math.sin(d_lng) * math.cos(lat2_rad)
    y = math.cos(lat1_rad) * math.sin(lat2_rad) - \
        math.sin(lat1_rad) * math.cos(lat2_rad) * math.cos(d_lng)

    bearing = math.degrees(math.atan2(x, y))
    return (bearing + 360) % 360

def angle_difference(a: float, b: float) -> float:
    """Shortest angular distance between two compass bearings (0–180)"""
    diff = abs(a - b) % 360
    return diff if diff <= 180 else 360 - diff

# ── LOOKUP FUNCTIONS ──────────────────────────────────────────────
def find_intersection(lat: float, lng: float):
    """Return the closest intersection if within INTERSECTION_RADIUS, else None"""
    closest = min(INTERSECTIONS, key=lambda i: abs(lat - i.lat) + abs(lng - i.lng))
    if (abs(lat - closest.lat) < INTERSECTION_RADIUS and
        abs(lng - closest.lng) < INTERSECTION_RADIUS):
        return closest
    return None

def find_segment(intersection: Intersection, angle: float):
    """
    Return the Segment whose exit angle is closest to `angle`.
    If the closest exit is more than MAX_ANGLE_MISMATCH degrees off, return None
    (meaning the robot turned onto an unmapped road).
    """
    closest_exit = min(intersection.exits, key=lambda e: angle_difference(angle, e.angle))
    if angle_difference(angle, closest_exit.angle) > MAX_ANGLE_MISMATCH:
        return None
    return closest_exit.segment

# ── MAIN DECISION ─────────────────────────────────────────────────
def make_decision(lat: float, lng: float, angle_delta: float, is_salting: bool, is_first_ping: bool) -> dict:

    # ── FIRST PING: save start position, do nothing else ─────────
    if is_first_ping:
        robot_state["start_lat"]  = lat
        robot_state["start_lng"]  = lng
        robot_state["calibrated"] = False
        return {"start_salting": False}

    # ── SECOND PING: calculate initial absolute heading ───────────
    if not robot_state["calibrated"]:
        # Make sure robot actually moved before calculating bearing
        lat_diff = abs(lat - robot_state["start_lat"])
        lng_diff = abs(lng - robot_state["start_lng"])
        if lat_diff < MIN_MOVEMENT_DEG and lng_diff < MIN_MOVEMENT_DEG:
            return {"start_salting": False}  # wait for actual movement

        bearing = calculate_bearing(
            robot_state["start_lat"], robot_state["start_lng"],
            lat, lng
        )
        robot_state["current_angle"] = (bearing + angle_delta) % 360
        robot_state["calibrated"]    = True

    # ── ALL SUBSEQUENT PINGS: update angle by delta ───────────────
    else:
        robot_state["current_angle"] = (robot_state["current_angle"] + angle_delta) % 360

    current_angle = robot_state["current_angle"]

    # ── INTERSECTION LOGIC ────────────────────────────────────────
    intersection = find_intersection(lat, lng)

    if intersection:
        segment = find_segment(intersection, current_angle)

        # Unmapped road (angle doesn't match any exit within 30°)
        # OR exit points to a segment_id not in SEGMENTS
        if segment is None:
            return {"start_salting": False}

        # Already salted within 8 hours
        if segment.is_salted():
            return {"start_salting": False}

        # Salt it
        segment.mark_salted()
        return {"start_salting": True}

    # Mid-street stop — maintain current salting state
    return {"start_salting": is_salting}

# ── DATABASE INIT ─────────────────────────────────────────────────
def init_db():
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()
    c.execute("""
        CREATE TABLE IF NOT EXISTS segments (
            segment_id TEXT PRIMARY KEY,
            salted_at  TEXT DEFAULT NULL
        )
    """)
    conn.commit()
    conn.close()
