"""
Behavioral state machine implementation for autonomous robot control.
Manages state transitions based on sensor input with hysteresis filtering
to prevent spurious state changes from sensor noise.
"""

from enum import Enum
from typing import Optional
from dataclasses import dataclass


class BehaviorState(Enum):
    FORAGE = "FORAGE"
    FREEZE = "FREEZE"
    FLEE = "FLEE"
    RECOVER = "RECOVER"


@dataclass
class SensorReading:
    distance_cm: float
    light_level: int
    battery_percent: int


class StateMachine:
    """
    Behavioral state machine with hysteresis-filtered transitions.
    Requires consistent sensor readings across multiple samples before
    changing state to prevent noise-induced state oscillation.
    """

    ALERT_THRESHOLD_CM = 150
    FLIGHT_THRESHOLD_CM = 80
    HYSTERESIS_COUNT = 3

    def __init__(self):
        self.current_state = BehaviorState.FORAGE
        self.reading_buffer = []
        self.recovery_start_time = None
        self.recovery_timeout_seconds = 30

    def update(self, reading: SensorReading) -> Optional[BehaviorState]:
        """
        Process sensor input and determine state transition.

        Returns new state if transition occurs, None if state unchanged.
        """
        self.reading_buffer.append(reading.distance_cm)
        if len(self.reading_buffer) > self.HYSTERESIS_COUNT:
            self.reading_buffer.pop(0)

        # Only consider state change if sufficient consistent readings
        if len(self.reading_buffer) < self.HYSTERESIS_COUNT:
            return None

        avg_distance = sum(self.reading_buffer) / len(self.reading_buffer)
        threat_detected = avg_distance < self.ALERT_THRESHOLD_CM
        immediate_threat = avg_distance < self.FLIGHT_THRESHOLD_CM

        new_state = self._compute_next_state(
            threat_detected, immediate_threat, reading
        )

        if new_state != self.current_state:
            self.current_state = new_state
            return new_state

        return None

    def _compute_next_state(
        self,
        threat_detected: bool,
        immediate_threat: bool,
        reading: SensorReading,
    ) -> BehaviorState:
        """Determine state based on sensor input and current state."""

        if self.current_state == BehaviorState.FORAGE:
            if immediate_threat:
                return BehaviorState.FLEE
            elif threat_detected:
                return BehaviorState.FREEZE

        elif self.current_state == BehaviorState.FREEZE:
            if immediate_threat:
                return BehaviorState.FLEE
            elif not threat_detected:
                return BehaviorState.FORAGE

        elif self.current_state == BehaviorState.FLEE:
            if not threat_detected:
                return BehaviorState.RECOVER

        elif self.current_state == BehaviorState.RECOVER:
            if immediate_threat:
                return BehaviorState.FLEE
            # Recovery timeout allows re-engagement after threat subsides

        return self.current_state
