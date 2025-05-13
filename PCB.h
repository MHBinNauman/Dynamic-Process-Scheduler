#ifndef PCB_H
#define PCB_H

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define NEW "new"
#define READY "ready"
#define WAITING "waiting"
#define RUNNING "running"
#define TERMINATED "terminated"


class PCB {
private:
    static int pidCounter;
    int pid;
    int ppid;
    vector<int> childPIDS;
    string processName;
    string processState;
    int arrivalTime;        // The time at which a process arrives and becomes ready to be executed
    int burstTime;          // The time for which the process needs the CPU
    int remainingTime;
    int waitingTime;
    int priority;
    int ioDeviceID;         // Track which I/O device is allocated to this process
    bool isCompleted;
    bool waitingForIO;

    void fork(int ppid, int current_time, int burst_time, int priority, bool waitingForIO) {
        this->pid = pidCounter++;          // Assigning a unique PID
        this->ppid = ppid;                 // Created by the program
        this->childPIDS.clear();           // No children at fork
        this->processName = "Process" + to_string(this->pid);
        this->processState = NEW;          // Initial state
        this->arrivalTime = current_time;
        this->burstTime = burst_time;
        this->waitingTime = 0;
        this->priority = priority;
        this->ioDeviceID = -1;             // No I/O device allocated initially
        this->isCompleted = false;
        this->waitingForIO = waitingForIO;
    }

public:
    PCB(int ppid, int current_time, int burst_time, int priority, bool waitingForIO) {
        fork(ppid, current_time, burst_time, priority, waitingForIO);
    }

    // Setters
    void setArrivalTime(int time) { arrivalTime = time; }
    void setBurstTime(int time) { burstTime = remainingTime = time; }
    void setRemainingTime(int currentTime) { this->remainingTime = max(0, (burstTime - (currentTime - arrivalTime))); }
    void setWaitingTime(int currentTime) { if (waitingForIO) { this->waitingTime = currentTime - this->arrivalTime; } }
    void setPriority(int p) { priority = p; }
    void setIODevicesAllocated(int deviceID) { 
        ioDeviceID = deviceID;
        waitingForIO = false;
    }
    void setWaitingForIO(bool status) { waitingForIO = status; }
    void completeProcess() { isCompleted = true; processState = TERMINATED; }
    void addChild(int childPID) { childPIDS.push_back(childPID); }

    // Getters
    int getPID() const { return this->pid; }
    int getPPID() const { return this->ppid; }
    const vector<int>& getChildPIDs() const { return this->childPIDS; }
    string getProcessName() const { return this->processName; }
    string getProcessState() const { return this->processState; }
    int getArrivalTime() const { return this->arrivalTime; }
    int getBurstTime() const { return this->burstTime; }
    int getRemainingTime() const { return this->remainingTime; }
    int getWaitingTime() const { return this->waitingTime; }
    int getPriority() const { return this->priority; }
    bool getIsCompleted() const { return this->isCompleted; }
    bool getWaitingForIO() const { return this->waitingForIO; }
    int getIODevicesAllocated() const { return ioDeviceID; }

    // Update functions
    void incrementWaitingTime() { ++this->waitingTime; }
    void decrementRemainingTime(int time) { this->remainingTime -= time; }
    void increasePriority() { ++priority; }

    // generate child processes
    PCB generateChildProcess(int current_time, int burst_time, int priority, bool waitingForIO) {
        PCB child(this->pid, current_time, burst_time, priority, waitingForIO);
        this->addChild(child.getPID());
        return child;
    }
    // update process state
    void updateProcessState(const string& newState) {
        if (newState == NEW || newState == READY || newState == WAITING ||
            newState == RUNNING || newState == TERMINATED) {
            processState = newState;
        }
        else {
            cerr << "Invalid process state: " << newState << endl;
        }
    }


};

int PCB::pidCounter = 0;


struct ComparePriority {
    bool operator()(const shared_ptr<PCB>& a, const shared_ptr<PCB>& b) {
        return a->getPriority() > b->getPriority();
    }
};

#endif
