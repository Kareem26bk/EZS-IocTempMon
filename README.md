# EPICS Real-Time Temperature Monitor

A C-based multithreaded simulation built on the **EPICS (Experimental Physics and Industrial Control System)** framework. This project demonstrates core real-time embedded systems concepts using the `libcom` OSI abstraction layer.

## Key Concepts Demonstrated
* **Producer-Consumer Pattern:** Utilizing `epicsMessageQueue` to safely pass data between hardware polling tasks and data processing tasks.
* **Thread Synchronization:** Using `epicsMutex` to protect shared memory states and prevent race conditions.
* **Event-Driven Architecture:** Implementing `epicsEvent` for a dormant, high-priority alarm thread that consumes 0% CPU until triggered by a critical interlock condition.
* **Rate Monotonic Scheduling:** Assigning thread priorities (0-99) based on hardware response requirements.
