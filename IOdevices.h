#ifndef IODEVICES_H
#define IODEVICES_H
#include <iostream>
using namespace std;

class IOdevices {
private:
	int deviceID;
	bool availability;
public:
	IOdevices(int Did) : availability(true), deviceID(Did) {}

	int getDeviceID() const {
		return this->deviceID;
	}

	void setAvailability(bool av) {
		this->availability = av;
	}

	bool getAvailability() const {
		return this->availability;
	}
};

#endif
