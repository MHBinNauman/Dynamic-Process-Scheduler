#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <queue>
#include <vector>
#include <memory>
#include "PCB.h"
#include "Scheduler.h"
#include "SimulationClock.h"
#include "PolicyEngine.h"
#include "IOdevices.h"
using namespace std;

class SimulationManager {
private:
	// System resources
	vector<IOdevices> ioDevices;  // true means device is available
	list<shared_ptr<PCB>> jobQueue;      
	priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> deviceQueue;
	priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> readyQueue;
	Scheduler* scheduler;
	SimulationClock clock;
	PolicyEngine* policyEngine;

	// System metrics structure
	struct SystemMetrics {
		double avgBurstTime;
		double percentHighPriority;
		int queueSize;
	};

	SystemMetrics calculateSystemMetrics() {
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

	// If the process needs IO devices and IO devices are not available then move it to device queue
	void allocateResources() {
		// Iterating over jobQueue
		for (auto it = jobQueue.begin(); it != jobQueue.end();) {

			if ((*it)->getWaitingForIO()) {
				bool ioAllocated = false;

				for (auto& ioDevice : ioDevices) {
					if (ioDevice.getAvailability()) {
						ioDevice.setAvailability(false);
						(*it)->setIODevicesAllocated(ioDevice.getDeviceID());
						(*it)->setWaitingForIO(false);
						(*it)->setArrivalTime(clock.getCurrentTime());
						(*it)->updateProcessState(READY);
					    readyQueue.push(*it);
						// Removing process from jobQueue
						cout << "Moved PID " << (*it)->getPID() << " from jobQueue to readyQueue (I/O Allocated)\n";
						it = jobQueue.erase(it);
						ioAllocated = true;
						break;  // Once IO device is allocated, break out of the loop
					}
				}

				// If IO devices are not available then pcb moved to deviceQueue
				if (!ioAllocated) {
					deviceQueue.push(*it);
					(*it)->setWaitingForIO(true);
					(*it)->updateProcessState(WAITING);
					cout << "Moved PID " << (*it)->getPID() << " to device queue (waiting for I/O)" << endl;

					// Safely remove from jobQueue and move to the next element
					it = jobQueue.erase(it);
				}
			}
			else {
				(*it)->setArrivalTime(clock.getCurrentTime());
				(*it)->updateProcessState(READY);
				readyQueue.push(*it);
				// Removing process from jobQueue
				cout << "Moved PID " << (*it)->getPID() << " to ready queue (no I/O needed)" << endl;
				it = jobQueue.erase(it);
			}
		}

		if (!deviceQueue.empty() && jobQueue.empty()) {
			priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> tempDeviceQueue;
			while (!deviceQueue.empty()) {
				bool ioAvailable = false;
				for (auto& device : ioDevices) {
					if (device.getAvailability()) {
						ioAvailable = true;
						device.setAvailability(false);
						deviceQueue.top()->setIODevicesAllocated(device.getDeviceID());
						break;
					}
				}

				if (ioAvailable) {
					deviceQueue.top()->setArrivalTime(clock.getCurrentTime());
					deviceQueue.top()->updateProcessState(READY);
					readyQueue.push(deviceQueue.top());
					cout << "Moved PID " << deviceQueue.top()->getPID() << " from deviceQueue to readyQueue\n";
					deviceQueue.pop();
				}
				else {
					cout << "No I/O device available for PID " << deviceQueue.top()->getPID() << endl;
					break;
				}
			}
		}
	}

	// Deallocate resources from completed processes
	void deallocateResources() {
		// Handle completed processes in readyQueue
		priority_queue<shared_ptr<PCB>, vector<shared_ptr<PCB>>, ComparePriority> tempReadyQueue;
		while (!readyQueue.empty()) {
			auto process = readyQueue.top();
			readyQueue.pop();

			if (process->getIsCompleted()) {
				int deviceID = process->getIODevicesAllocated();
				if (deviceID != -1) {
					for (auto& device : ioDevices) {
						if (device.getDeviceID() == deviceID) {
							device.setAvailability(true);
							cout << "Released I/O device " << device.getDeviceID() << " from completed process " << process->getPID() << endl;
							break;
						}
					}
				}
				cout << "Process " << process->getPID() << " completed and deallocated.\n";
			}
			else {
				tempReadyQueue.push(process);
			}
		}
		readyQueue = tempReadyQueue;
	}

	// loads processes from a file into jobQueue
	void loadProcesses() {
		string processes;

		// Reading the program
		ifstream programReader("Program.txt");
		if (!programReader.is_open()) {
			cerr << "Failed to open process file: file.txt" << endl;
		}

		while (getline(programReader, processes)) {
			try {
				PCB process = createPCBFromProgram(processes);
				shared_ptr<PCB> ptr_to_PCB = make_shared<PCB>(process);
				jobQueue.push_back(ptr_to_PCB);
			} catch (const exception& e) {
				cerr << "Error loading process: " << e.what() << endl;
			}
		}

		// Close the file
		programReader.close();
	}

	PCB createPCBFromProgram(string processes){
		int ppid, current_time, burst_time, priority;
		bool waitingForIO;
		// Parse line and create PCB
		// Implementation depends on your process file format
		stringstream ss(processes);
		ss >> ppid >> burst_time >> priority >> waitingForIO;
		current_time = clock.getCurrentTime();
		PCB process(ppid, current_time, burst_time, priority, waitingForIO);
		process.setRemainingTime(burst_time);  // Set remaining time equal to burst time
		return process;
	}

	void loadIOdevices() {
		int id = 1000;
		for (int i = 0; i < 20; i++) {
			IOdevices device(id);
			id++;
			ioDevices.push_back(device);
		}
	}

public:
	SimulationManager() {
		scheduler = new Scheduler(&clock);
		policyEngine = new PolicyEngine(scheduler);
		loadProcesses();
		loadIOdevices();
	}

	void simulateScheduling() {
		cout << "------------------------------------" << endl;
		cout << "#### Process Scheduling Started ####" << endl;
		cout << "------------------------------------" << endl;
		
		while(!jobQueue.empty() || !readyQueue.empty() || !deviceQueue.empty()) {
			// Current queue states bfr allocation of resources
			cout << "#### Queue Status ####" << endl;
			cout << "Job Queue Size: " << jobQueue.size() << endl;
			cout << "Ready Queue Size: " << readyQueue.size() << endl;
			cout << "Device Queue Size: " << deviceQueue.size() << endl;

			// Allocate Resources
			cout << "Allocating Resources ... " << endl;
			
			allocateResources();

			// Current queue states after allocation of resources
			cout << "#### Queue Status ####" << endl;
			cout << "Job Queue Size: " << jobQueue.size() << endl;
			cout << "Ready Queue Size: " << readyQueue.size() << endl;
			cout << "Device Queue Size: " << deviceQueue.size() << endl;
			
			cout << "CPU Scheduling Phase ... " << endl;
			
			// This loop schedules CPU
			while(!readyQueue.empty()) {
				
				policyEngine->chooseSchedulingAlgorithm(readyQueue, 5);
				cout << "Deallocating Resources from completed processes ... " << endl;
				deallocateResources();
			}
			
			// Advancing the simulation clock
			clock.tick();
		}
		
		cout << "---------------------------------" << endl;
		cout << "#### Scheduler Shutting Down ####" << endl;
		cout << "---------------------------------" << endl;
	}
};

#endif