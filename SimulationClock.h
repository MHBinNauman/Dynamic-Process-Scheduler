#ifndef SIMULATIONCLOCK_H
#define SIMULATIONCLOCK_H
#include <iostream>
using namespace std;

class SimulationClock {
private:
	int currentTime;

public:
	SimulationClock() : currentTime(0) {}

	void tick() {
		currentTime++;
	}

	void tick(int time) {
		currentTime += time;
	}

	int getCurrentTime() const {
		return currentTime;
	}

	void reset() {
		currentTime = 0;
	}
};

#endif
