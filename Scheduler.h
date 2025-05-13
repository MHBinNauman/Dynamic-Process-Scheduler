#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <queue>
#include <list>
#include <vector>
#include <deque>
#include <algorithm>
#include "PCB.h"
#include "SimulationClock.h"
using namespace std;

class Scheduler {
private:
    SimulationClock* clock;

    // copies the main list in case of RR to run any process from anywhere in vector which couldnt be done in queue 
    vector<shared_ptr<PCB>> snapshotQueue(const priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& q) {
        vector<shared_ptr<PCB>> vec;
        auto temp = q;
        while (!temp.empty()) {
            vec.push_back(temp.top());
            temp.pop();
        }
        return vec;
    }


public:
    Scheduler(SimulationClock* clk) : clock(clk) {}

    void runFCFS(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue, bool stopFlag) 
    {
        for (auto& process : snapshotQueue(readyQueue)) {
            if (stopFlag || process->getIsCompleted()) return; //if stopflag = 1 ... just return

            process->updateProcessState(RUNNING);
            int execTime = process->getRemainingTime();

            cout << "[FCFS] PID " << process->getPID() << " executing for " << execTime << "ms\n";
            clock->tick(execTime);
            process->setRemainingTime(0);
            process->completeProcess();

            cout << "[FCFS] PID " << process->getPID() << " completed at time " << clock->getCurrentTime() << "ms\n";
            clock->tick();
        }
    }

    void runPriority(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue, bool stopFlag) 
    {
        if (!stopFlag) {
            priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> tempQueue;
            while (!readyQueue.empty()) {
                auto process = readyQueue.top();
                readyQueue.pop();

                if (stopFlag || process->getIsCompleted()) continue;

                process->updateProcessState(RUNNING);
                int execTime = process->getRemainingTime();

                cout << "[PRIO] PID " << process->getPID() << " executing for " << execTime << "ms\n";
                clock->tick(execTime);
                process->setRemainingTime(0);
                process->completeProcess();

                cout << "[PRIO] PID " << process->getPID() << " completed at time " << clock->getCurrentTime() << "ms\n";

                // Update the process in the tempQueue
                if (!process->getIsCompleted()) {
                    tempQueue.push(process);
                }
                clock->tick();
            }

            // Swap back the updated queue
            readyQueue = tempQueue;
        }
    }


    void runSJF(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue, bool stopFlag) 
    {
        auto processes = snapshotQueue(readyQueue);
        // to sort processes list according to brust time
        sort(processes.begin(), processes.end(), [](const auto& a, const auto& b) { 
            return a->getBurstTime() < b->getBurstTime();
            });

        for (auto& proc : processes) {
            if (stopFlag || proc->getIsCompleted()) return;

            proc->updateProcessState(RUNNING);
            int execTime = proc->getRemainingTime();

            cout << "[SJF] PID " << proc->getPID() << " executing for " << execTime << "ms\n";
            clock->tick(execTime);
            proc->setRemainingTime(0);
            proc->completeProcess();

            cout << "[SJF] PID " << proc->getPID() << " completed at time " << clock->getCurrentTime() << "ms\n";
            clock->tick();
        }
    }

    void runRoundRobin(priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority>& readyQueue, bool stopFlag, int timeQuantum) 
    {
        deque<shared_ptr<PCB>> rrQueue;
        priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> temp = readyQueue;
        // copying the priority_queue to the double ended queue
        while (!temp.empty()) {
            rrQueue.push_back(temp.top());
            temp.pop();
        }

        while (!rrQueue.empty() && !stopFlag) {
            auto proc = rrQueue.front(); rrQueue.pop_front();
            if (proc->getIsCompleted()) continue;

            int execTime = min(proc->getRemainingTime(), timeQuantum);
            proc->updateProcessState(RUNNING);

            cout << "[RR] PID " << proc->getPID() << " running for " << execTime << "ms\n";
            clock->tick(execTime);
            clock->tick();
            proc->decrementRemainingTime(execTime);

            if (proc->getRemainingTime() == 0) {
                proc->completeProcess();
                cout << "[RR] PID " << proc->getPID() << " completed\n";
            }
            else {
                proc->updateProcessState(READY);
                rrQueue.push_back(proc);
            }
        }
    }
};

#endif
