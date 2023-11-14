enum TrafficLightState { RED, YELLOW, GREEN };
enum TrafficLightState currentState = RED;

// In a loop or based on a timer, you could transition like this:
if (currentState == RED) {
    // Transition to Green after a certain time
    currentState = GREEN;
} else if (currentState == GREEN) {
    // Transition to Yellow, and then Red
    currentState = YELLOW;
} else if (currentState == YELLOW) {
    // Transition to Red
    currentState = RED;
}
