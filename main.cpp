#include "SimulationManager.h"
#include <iostream>
using namespace std;

int main() {
    try {
        cout << "Starting Process Scheduler Simulation...\n" << endl;
        
        SimulationManager simulation;
    
        simulation.simulateScheduling();
        
        cout << "\nSimulation completed successfully!" << std::endl;
    }
    catch (const exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        return 1;
    }

    return 0;
}
