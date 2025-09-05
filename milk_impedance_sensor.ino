/**
* milk_impedance_sensor.ino
* ad5933-test
 * Reads impedance values from the AD5933 over I2C and prints them serially.
 *
 * Edited by Jeanette Qi, Bryant Pham, and Sejad Mousa
 *
 * Last updated 12/20/2024
 */

#include <Wire.h>
#include "AD5933.h"

#define START_FREQ  (100)  // Start frequency in Hz (50 kHz)
#define FREQ_INCR   (100)    // Frequency increment in Hz
#define NUM_INCR    (100)     // Number of frequency points in the sweep
#define REF_RESIST  (330)  // Reference resistor value in ohms for calibration

#define ALPHA 0.1 // Low-pass filter smoothing factor (0.0 < ALPHA < 1.0)

float gain[NUM_INCR + 1];
int phase[NUM_INCR + 1];
// double impedance_filtered[NUM_INCR + 1]; // Stores filtered impedance

// To change the number of iterations, set numIterations to any value that you want
const int numIterations = 1;
int loopCounter = 1; // Counter to track iterations
bool stopLoop = false;

void setup(void) {
    // Begin I2C
    Wire.begin();

    // Begin serial at 9600 baud for output
    Serial.begin(9600); //115200
    Serial.println("AD5933 Configuration Started");

    delay(1000);

    // Initialize gain[] array
    for (int i = 0; i < NUM_INCR + 1; i++) {
        gain[i] = 0.0;
        //impedance_filtered[i]=0.0;
        //Serial.println("gain array");
    }

    // Validate reference resistance
    if (REF_RESIST <= 0) {
        Serial.println("Error: Invalid reference resistance. Check REF_RESIST.");
        while (true); // Halt execution
    }


    // Perform initial configuration
    if (!(AD5933::reset() &&
          AD5933::setInternalClock(true) &&
          AD5933::setStartFrequency(START_FREQ) &&
          AD5933::setIncrementFrequency(FREQ_INCR) &&
          AD5933::setNumberIncrements(NUM_INCR) &&
          AD5933::setPGAGain(PGA_GAIN_X1) && 
          AD5933::setRange(CTRL_OUTPUT_RANGE_4))){
        Serial.println("FAILED in initialization!");
        while (true); // Halt execution
    }

    // Perform calibration sweep
    Serial.println("Starting calibration sweep...");
    if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR + 1)) {
        Serial.println("----------------------------------Calibrated!");

       /* // Debugging: Print all gain values after calibration
        Serial.println("Calibration Gains:");
        for (int i = 0; i < NUM_INCR + 1; i++) {
            Serial.print("Gain[");
            Serial.print(i);
            Serial.print("]: ");
            Serial.println(gain[i], 12);
        }*/
    } else {
        Serial.println("----------------------------------Calibration failed...");
    }

// Prompt user to insert test impedance
    Serial.println("----------------------------------Insert test impedance, then press 'y' and Enter to continue");

    // Clear any previous serial input
    while (Serial.available() > 0) {
        Serial.read();
    }

    // Accumulate serial input into buffer
    String inputBuffer = "";

    // Wait for 'y' confirmation from Python or Serial Monitor
    while (true) {
        while (Serial.available() > 0) {
            char inputChar = Serial.read();

            if (inputChar == '\n' || inputChar == '\r') {
                inputBuffer.trim();  // clean newline/carriage return

                if (inputBuffer.equalsIgnoreCase("y")) {
                    Serial.println("Continuing with measurement...");
                    return; // Exit setup()
                } else {
                    Serial.println("Invalid input, type 'y' to continue.");
                    inputBuffer = ""; // reset 
                }
            } else {
                inputBuffer += inputChar;
            }
        }
    }
}
    //boolean flag = true;
    //char receivedChar;
    //while (flag) {
    //    if (Serial.available() > 0) {
    //        receivedChar = Serial.read();
    //        flag = false;
    //    } 
    //}
    // if(user input)
    // leave while loop

//}


void loop(void) {
    if (!stopLoop) {
        Serial.println("----------------------------------Iteration: " + String(loopCounter));

        // Perform a frequency sweep using the easy method
        Serial.println("----------------------------------FS - Easy Begin----------------------------------");
        frequencySweepEasy();
        Serial.println("----------------------------------FS  -  Easy End----------------------------------");

        // Delay between iterations
        delay(3000);

        loopCounter++;

        if (loopCounter > numIterations) {
            stopLoop = true; // Set flag to stop the loop
        }
    } else {
        Serial.println("----------------------------------Finished executing the loop.");
        Serial.println("----------------------------------Close serial connection (CTRL+C)");
        while (true); // Halt execution
    }
}

void frequencySweepEasy() {
    // Create arrays to hold the data
    int real[NUM_INCR + 1], imag[NUM_INCR + 1];

    // Perform the frequency sweep
    if (AD5933::frequencySweep(real, imag, NUM_INCR + 1)) {
        // Start frequency in kHz
        double cfreq = START_FREQ / 1000.0;

        // Loop through the frequency increments
        for (int i = 0; i < NUM_INCR + 1; i++, cfreq += FREQ_INCR / 1000.0) {
            // Compute magnitude and impedance
            double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
            double impedance = (gain[i] > 0) ? (1 / (magnitude * gain[i])) : 0; // check if gain > 0, if so calculate, if not imp=0 

            // Calculate scaled gain
            //double scaled_gain = gain[i] * 1e9;
            double scaled_gain = gain[i];
            // Apply low-pass filter
            //if (i == 0) {
            //    impedance_filtered[i] = impedance; // No filtering for the first value
            //} else {
               // impedance_filtered[i] = ALPHA * impedance + (1 - ALPHA) * impedance_filtered[i - 1];
            //}


            // Calculate phase
            //double phase = atan2((double)imag[i], (double)real[i]) * (180.0 / PI);
            double phase = 0;
            if (real[i] == 0 && imag[i] == 0) {
                phase = NAN; // Handle undefined phase
            } else {
                phase = atan2((double)imag[i], (double)real[i]) * (180.0 / PI);

                // Normalize to [0°, 360°]
                if (phase < 0) {
                    phase += 360.0;
                }

            }
            // Print results
            Serial.print(String(loopCounter));
            Serial.print(",frequency_sweep_easy");
            Serial.print("," + String(cfreq, 3));         // Frequency in kHz
            Serial.print("," + String(real[i]));         // Real part
            Serial.print("," + String(imag[i]));         // Imaginary part
            Serial.print("," + String(scaled_gain, 12));  // Scaled Gain (x10^9) //6
            Serial.print("," + String(magnitude));       // Magnitude
            Serial.print("," + String(impedance, 6));  // Impedance
            //Serial.print("," + String(impedance_filtered[i], 6));
            Serial.println("," + String(phase, 6));  // Phase #Added

        }

        Serial.println("Frequency sweep success! Wait for loop to close.");
    } else {
        Serial.println("Frequency sweep failed...");
    }
        delay(3000); // Delay before the next loop iteration

}

// Removes the frequencySweep abstraction from above. This saves memory and
// allows for data to be processed in real time. However, it's more complex.
void frequencySweepRaw() {
    // Create variables to hold the impedance data and track frequency
    int real, imag, i = 0;
    double cfreq = START_FREQ / 1000.0;

    // Initialize the frequency sweep
    if (!(AD5933::setPowerMode(POWER_STANDBY) &&          // place in standby
          AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
          AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
    {
        Serial.println("Could not initialize frequency sweep...");
        return;
    }

    // Perform the actual sweep
    while ((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
        // Get the frequency data for this frequency point
        if (!AD5933::getComplexData(&real, &imag)) {
            Serial.println("Could not get raw frequency data...");
            continue;
        }

        // Compute magnitude and impedance
        double magnitude = sqrt(pow(real, 2) + pow(imag, 2));
        double impedance = (gain[i] > 0) ? (1 / (magnitude * gain[i])) : 0;

        // Calculate gain based on real and imaginary components
        //double scaled_gain = gain[i] * 1e9;
        double scaled_gain = gain[i]; //save as decimal, not scaled version

        double phase;
        if (real == 0 && imag == 0) {
            phase = 0;  // Handle edge case
        } else {
            phase = atan2(imag, real) * (180.0 / PI);
        }

        // Print results
        Serial.print(String(loopCounter));
        Serial.print(",frequency_sweep_raw");
        Serial.print("," + String(cfreq, 3));        // Frequency in kHz
        Serial.print("," + String(real));            // Real part
        Serial.print("," + String(imag));            // Imaginary part
        Serial.print("," + String(scaled_gain, 12)); // Scaled Gain (x10^9)
        Serial.print("," + String(magnitude));       // Magnitude
        Serial.print("," + String(impedance, 6));    // Impedance
        //Serial.print("," + String(impedance_filtered[i], 6));
        Serial.println("," + String(phase, 6));      // Phase

        // Increment the frequency
        i++;
        cfreq += FREQ_INCR / 1000.0;
        AD5933::setControlMode(CTRL_INCREMENT_FREQ);
    }

    Serial.println("Frequency sweep complete!");

    // Set AD5933 power mode to standby when finished
    if (!AD5933::setPowerMode(POWER_STANDBY))
        Serial.println("Could not set to standby...");


}

 #include <Wire.h>
 #include "AD5933.h"
 
 #define START_FREQ  (100)  // Start frequency in Hz (50 kHz)
 #define FREQ_INCR   (100)    // Frequency increment in Hz
 #define NUM_INCR    (100)     // Number of frequency points in the sweep
 #define REF_RESIST  (330)  // Reference resistor value in ohms for calibration
 
 #define ALPHA 0.1 // Low-pass filter smoothing factor (0.0 < ALPHA < 1.0)
 
 float gain[NUM_INCR + 1];
 int phase[NUM_INCR + 1];
 // double impedance_filtered[NUM_INCR + 1]; // Stores filtered impedance
 
 // To change the number of iterations, set numIterations to any value that you want
 const int numIterations = 1;
 int loopCounter = 1; // Counter to track iterations
 bool stopLoop = false;
 
 void setup(void) {
     // Begin I2C
     Wire.begin();
 
     // Begin serial at 9600 baud for output
     Serial.begin(9600); //115200
     Serial.println("AD5933 Configuration Started");
 
     delay(1000);
 
     // Initialize gain[] array
     for (int i = 0; i < NUM_INCR + 1; i++) {
         gain[i] = 0.0;
         //impedance_filtered[i]=0.0;
         //Serial.println("gain array");
     }
 
     // Validate reference resistance
     if (REF_RESIST <= 0) {
         Serial.println("Error: Invalid reference resistance. Check REF_RESIST.");
         while (true); // Halt execution
     }
 
 
     // Perform initial configuration
     if (!(AD5933::reset() &&
           AD5933::setInternalClock(true) &&
           AD5933::setStartFrequency(START_FREQ) &&
           AD5933::setIncrementFrequency(FREQ_INCR) &&
           AD5933::setNumberIncrements(NUM_INCR) &&
           AD5933::setPGAGain(PGA_GAIN_X1) && 
           AD5933::setRange(CTRL_OUTPUT_RANGE_4))){
         Serial.println("FAILED in initialization!");
         while (true); // Halt execution
     }
 
     // Perform calibration sweep
     Serial.println("Starting calibration sweep...");
     if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR + 1)) {
         Serial.println("----------------------------------Calibrated!");
 
        /* // Debugging: Print all gain values after calibration
         Serial.println("Calibration Gains:");
         for (int i = 0; i < NUM_INCR + 1; i++) {
             Serial.print("Gain[");
             Serial.print(i);
             Serial.print("]: ");
             Serial.println(gain[i], 12);
         }*/
     } else {
         Serial.println("----------------------------------Calibration failed...");
     }
 
 // Prompt user to insert test impedance
     Serial.println("----------------------------------Insert test impedance, then press 'y' and Enter to continue");
 
     // Clear any previous serial input
     while (Serial.available() > 0) {
         Serial.read();
     }
 
     // Accumulate serial input into buffer
     String inputBuffer = "";
 
     // Wait for 'y' confirmation from Python or Serial Monitor
     while (true) {
         while (Serial.available() > 0) {
             char inputChar = Serial.read();
 
             if (inputChar == '\n' || inputChar == '\r') {
                 inputBuffer.trim();  // clean newline/carriage return
 
                 if (inputBuffer.equalsIgnoreCase("y")) {
                     Serial.println("Continuing with measurement...");
                     return; // Exit setup()
                 } else {
                     Serial.println("Invalid input, type 'y' to continue.");
                     inputBuffer = ""; // reset 
                 }
             } else {
                 inputBuffer += inputChar;
             }
         }
     }
 }
     //boolean flag = true;
     //char receivedChar;
     //while (flag) {
     //    if (Serial.available() > 0) {
     //        receivedChar = Serial.read();
     //        flag = false;
     //    } 
     //}
     // if(user input)
     // leave while loop
 
 //}
 
 
 void loop(void) {
     if (!stopLoop) {
         Serial.println("----------------------------------Iteration: " + String(loopCounter));
 
         // Perform a frequency sweep using the easy method
         Serial.println("----------------------------------FS - Easy Begin----------------------------------");
         frequencySweepEasy();
         Serial.println("----------------------------------FS  -  Easy End----------------------------------");
 
         // Delay between iterations
         delay(3000);
 
         loopCounter++;
 
         if (loopCounter > numIterations) {
             stopLoop = true; // Set flag to stop the loop
         }
     } else {
         Serial.println("----------------------------------Finished executing the loop.");
         Serial.println("----------------------------------Close serial connection (CTRL+C)");
         while (true); // Halt execution
     }
 }
 
 void frequencySweepEasy() {
     // Create arrays to hold the data
     int real[NUM_INCR + 1], imag[NUM_INCR + 1];
 
     // Perform the frequency sweep
     if (AD5933::frequencySweep(real, imag, NUM_INCR + 1)) {
         // Start frequency in kHz
         double cfreq = START_FREQ / 1000.0;
 
         // Loop through the frequency increments
         for (int i = 0; i < NUM_INCR + 1; i++, cfreq += FREQ_INCR / 1000.0) {
             // Compute magnitude and impedance
             double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
             double impedance = (gain[i] > 0) ? (1 / (magnitude * gain[i])) : 0; // check if gain > 0, if so calculate, if not imp=0 
 
             // Calculate scaled gain
             //double scaled_gain = gain[i] * 1e9;
             double scaled_gain = gain[i];
             // Apply low-pass filter
             //if (i == 0) {
             //    impedance_filtered[i] = impedance; // No filtering for the first value
             //} else {
                // impedance_filtered[i] = ALPHA * impedance + (1 - ALPHA) * impedance_filtered[i - 1];
             //}
 
 
             // Calculate phase
             //double phase = atan2((double)imag[i], (double)real[i]) * (180.0 / PI);
             double phase = 0;
             if (real[i] == 0 && imag[i] == 0) {
                 phase = NAN; // Handle undefined phase
             } else {
                 phase = atan2((double)imag[i], (double)real[i]) * (180.0 / PI);
 
                 // Normalize to [0°, 360°]
                 if (phase < 0) {
                     phase += 360.0;
                 }
 
             }
             // Print results
             Serial.print(String(loopCounter));
             Serial.print(",frequency_sweep_easy");
             Serial.print("," + String(cfreq, 3));         // Frequency in kHz
             Serial.print("," + String(real[i]));         // Real part
             Serial.print("," + String(imag[i]));         // Imaginary part
             Serial.print("," + String(scaled_gain, 12));  // Scaled Gain (x10^9) //6
             Serial.print("," + String(magnitude));       // Magnitude
             Serial.print("," + String(impedance, 6));  // Impedance
             //Serial.print("," + String(impedance_filtered[i], 6));
             Serial.println("," + String(phase, 6));  // Phase #Added
 
         }
 
         Serial.println("Frequency sweep success! Wait for loop to close.");
     } else {
         Serial.println("Frequency sweep failed...");
     }
         delay(3000); // Delay before the next loop iteration
 
 }
 
 // Removes the frequencySweep abstraction from above. This saves memory and
 // allows for data to be processed in real time. However, it's more complex.
 void frequencySweepRaw() {
     // Create variables to hold the impedance data and track frequency
     int real, imag, i = 0;
     double cfreq = START_FREQ / 1000.0;
 
     // Initialize the frequency sweep
     if (!(AD5933::setPowerMode(POWER_STANDBY) &&          // place in standby
           AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
           AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
     {
         Serial.println("Could not initialize frequency sweep...");
         return;
     }
 
     // Perform the actual sweep
     while ((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
         // Get the frequency data for this frequency point
         if (!AD5933::getComplexData(&real, &imag)) {
             Serial.println("Could not get raw frequency data...");
             continue;
         }
 
         // Compute magnitude and impedance
         double magnitude = sqrt(pow(real, 2) + pow(imag, 2));
         double impedance = (gain[i] > 0) ? (1 / (magnitude * gain[i])) : 0;
 
         // Calculate gain based on real and imaginary components
         //double scaled_gain = gain[i] * 1e9;
         double scaled_gain = gain[i]; //save as decimal, not scaled version
 
         double phase;
         if (real == 0 && imag == 0) {
             phase = 0;  // Handle edge case
         } else {
             phase = atan2(imag, real) * (180.0 / PI);
         }
 
         // Print results
         Serial.print(String(loopCounter));
         Serial.print(",frequency_sweep_raw");
         Serial.print("," + String(cfreq, 3));        // Frequency in kHz
         Serial.print("," + String(real));            // Real part
         Serial.print("," + String(imag));            // Imaginary part
         Serial.print("," + String(scaled_gain, 12)); // Scaled Gain (x10^9)
         Serial.print("," + String(magnitude));       // Magnitude
         Serial.print("," + String(impedance, 6));    // Impedance
         //Serial.print("," + String(impedance_filtered[i], 6));
         Serial.println("," + String(phase, 6));      // Phase
 
         // Increment the frequency
         i++;
         cfreq += FREQ_INCR / 1000.0;
         AD5933::setControlMode(CTRL_INCREMENT_FREQ);
     }
 
     Serial.println("Frequency sweep complete!");
 
     // Set AD5933 power mode to standby when finished
     if (!AD5933::setPowerMode(POWER_STANDBY))
         Serial.println("Could not set to standby...");
 
 
 }
 
