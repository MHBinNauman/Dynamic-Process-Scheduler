#ifndef POLICYENGINE_H
#define POLICYENGINE_H
#include <iostream>
#include <queue>
#include <memory>
#include "PCB.h"
#include "Scheduler.h"
using namespace std;

// System metrics structure
struct SystemMetrics {
    double avgBurstTime;
    double percentHighPriority;
    int queueSize;
};

class PolicyEngine {
private:
    Scheduler* scheduler;
    const int AGING_THRESHOLD = 10;            
    const int MAX_PRIORITY = 1;                
    const int PRIORITY_INC = 1;                 // How much to increase priority by
    const int THRESH_QUEUE_SIZE = 20;           
    const double THRESH_BURST_TIME = 10.0;       
    const double THRESH_HIGH_PRIORITY = 45.0; 

    SystemMetrics calculateSystemMetrics(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue) {
        SystemMetrics metrics;
        metrics.queueSize = readyQueue.size();

        double totalBurstTime = 0;
        int highPriorityCount = 0;
        int totalProcesses = 0;

        priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> tempQueue = readyQueue;
        while (!tempQueue.empty()) {
            auto process = tempQueue.top();
            totalBurstTime += process->getBurstTime();
            if (process->getPriority() < 2) highPriorityCount++;
            totalProcesses++;
            tempQueue.pop();
        }

        metrics.avgBurstTime = totalProcesses > 0 ? totalBurstTime / totalProcesses : 0;
        metrics.percentHighPriority = totalProcesses > 0 ? (highPriorityCount * 100.0) / totalProcesses : 0;

        return metrics;
    }

    void applyAging(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue) {
        priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> tempQueue;
        
        while (!readyQueue.empty()) {
            shared_ptr<PCB> process = readyQueue.top();
            readyQueue.pop();
            
            process->setWaitingTime(process->getWaitingTime() + 1);
            
            if (process->getWaitingTime() + 1 > AGING_THRESHOLD) {
                int currentPriority = process->getPriority();
                if (currentPriority > MAX_PRIORITY) {   // checking priority
                    int newPriority = max(MAX_PRIORITY, currentPriority - PRIORITY_INC);
                    process->setPriority(newPriority);
                    process->setWaitingTime(0);         // because aging is implemented
                }
            }
            tempQueue.push(process);
        }
        
        readyQueue = tempQueue;
    }

public:
    PolicyEngine(Scheduler* sch) : scheduler(sch) {}

    void chooseSchedulingAlgorithm(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue,
         int timeQuantum) {
        // Applying aging bfr choosing algo; avoiding starvation
        applyAging(readyQueue);
        SystemMetrics metrics = calculateSystemMetrics(readyQueue);	// Calc sys metrics for scheduling
        cout << "Avg Burst Time: " << metrics.avgBurstTime << "\nQueue Size: " << metrics.queueSize << "\n% High Priority: " << metrics.percentHighPriority << endl;
        
        if (metrics.queueSize > THRESH_QUEUE_SIZE) {
            cout << "Running Round Robin Scheduling ..." << endl;
            scheduler->runRoundRobin(readyQueue, 0, timeQuantum);
            scheduler->runSJF(readyQueue, 1);
            scheduler->runPriority(readyQueue, 1);
            scheduler->runFCFS(readyQueue, 1);
        }
        else if (metrics.avgBurstTime < THRESH_BURST_TIME) {
            cout << "Running SJF Scheduling ..." << endl;
            scheduler->runSJF(readyQueue, 0);
            scheduler->runRoundRobin(readyQueue, 1, timeQuantum);
            scheduler->runPriority(readyQueue, 1);
            scheduler->runFCFS(readyQueue, 1);
        }
        else if (metrics.percentHighPriority > THRESH_HIGH_PRIORITY) {
            cout << "Running Priority Scheduling ..." << endl;
            scheduler->runPriority(readyQueue, 0);
            scheduler->runSJF(readyQueue, 1);
            scheduler->runRoundRobin(readyQueue, 1, timeQuantum);
            scheduler->runFCFS(readyQueue, 1);
        }
        else {
            cout << "Running FCFS Scheduling ..." << endl;
            scheduler->runFCFS(readyQueue, 0);
            scheduler->runSJF(readyQueue, 1);
            scheduler->runRoundRobin(readyQueue, 1, timeQuantum);
            scheduler->runPriority(readyQueue, 1);
        }
    }
};

#endif
