#include "iface.h"
// RX pipeline which runs on a seperate thread.
// Handles messages asynchronously compared to TX pipeline which relies on "send" calls from application

/**
 * Starts the recieve pipeline. Allows stack to asynchronously handle incoming ethernet frames to interface.
 */
void start_rx(iface_t interface);
/**
 * Stops the recieve pipeline
 */
void stop_rx();