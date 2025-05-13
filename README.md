# Dynamic Process Scheduler

A C++ implementation of a dynamic process scheduling simulator. This project dynamically switches between scheduling algorithms based on system load and user-defined policies, ensuring efficient CPU utilization and reduced context switching.

## Features

- Supports multiple scheduling algorithms: FCFS, SJF, Priority, Round Robin
- Dynamic algorithm switching based on system metrics
- Process state management: NEW, READY, RUNNING, WAITING, TERMINATED
- I/O device management and resource allocation
- Real-time metrics calculation (average burst time, high-priority percentage)
- Simulation clock for time management

## Class Structure

### SimulationManager

- Orchestrates the scheduling simulation
- Manages queues and allocates/deallocates resources
- Loads processes from input file

### Scheduler

- Manages process scheduling based on selected algorithm
- Switches algorithms dynamically as per system load
- Dispatches processes from ready queue to CPU

### PolicyEngine

- Determines optimal scheduling algorithm based on system metrics
- Switches between FCFS, SJF, Priority, and Round Robin

### PCB (Process Control Block)

- Represents individual processes
- Stores attributes like PID, burst time, priority, and state
- Manages process state transitions

### IOdevices

- Manages I/O devices and availability
- Allocates and deallocates devices to processes

### SimulationClock

- Tracks simulation time
- Manages clock ticks and time progression

## Console Output

![Console Output](static/project-output.gif)
