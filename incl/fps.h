#pragma once
 
#include <iostream>
#include <iomanip>
#include <sstream>
 
/**
 * @brief Class for calculating Frames Per Second (FPS).
 *
 * This class provides functionality for calculating the Frames Per Second (FPS)
 * based on the time elapsed between consecutive frames.
 */
class FPSCounter {
private:
    // Member variables for FPS calculation

    float prevFPS;      /**< Previous FPS value */
    double prevTime;    /**< Previous time */
    double crntTime;    /**< Current time */
    double timeDiff;    /**< Time difference between frames */
    unsigned int counter; /**< Frame counter */

    static FPSCounter* p_instance; /**< Singleton instance */

    // Private constructor to prevent instantiation
    FPSCounter(){ x_reset(); }

    // Reset method to reset FPS calculation
    void x_reset() {
        prevFPS = 60.f;
        prevTime = 0.;
        crntTime = 0.;
        timeDiff = 0.;
        counter = 0;
    }

public:
    // Singleton getter
    static FPSCounter* getInstance() {
        return p_instance;
    }

    // Delete copy constructor, move, and assignment operator

    FPSCounter(const FPSCounter&) = delete; /**< Copy constructor */
    FPSCounter(const FPSCounter&&) = delete; /**< Move constructor */
    FPSCounter& operator=(const FPSCounter&) = delete; /**< assignment operator */
    FPSCounter& operator=(const FPSCounter&&) = delete; /**< Move assignment operator */


    /**
     * @brief Reset the FPS calculation.
     */
    void reset() {
        x_reset();
    }
    /**
     * @brief Start the FPS calculation timer.
     */
    void start() {
        prevTime = glfwGetTime();
        return;
    }

    /**
     * @brief Update the FPS calculation.
     *
     * This method updates the FPS calculation based on the time elapsed between
     * consecutive frames.
     *
     * @return float Calculated FPS value
     */
    auto update()-> float
    { 

        // Initialize FPS with previous value
        float fps = prevFPS;

        // Get current time
        double _crntTime = glfwGetTime();

        if (_crntTime > 0.) {
            crntTime = _crntTime;

            // Calculate time difference
            timeDiff = crntTime - prevTime;

            // Increment frame counter
            counter++;

            // Update FPS if time interval is more than .1 sec
            if (timeDiff >= 1. / 10.) {
                
                fps = static_cast<float>(1.0f / timeDiff) * counter;

                // Reset time and counter for next interval
                prevTime = crntTime;
                counter = 0;
                
                // Update previous FPS
                prevFPS = fps;
           
            } 
        }
        return fps;
    }
};

// Initializing <p_instance> with an instance
FPSCounter* FPSCounter::p_instance = new FPSCounter();