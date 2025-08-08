**Set up**

**Download these files from Github:**
1. Read_serial_data.py
2. Plot_data.py
3. Requirements.txt


**Setup Environment**
1. Download VS Code
2. Download latest Python 3.XXX
3. Create python virtual environment using the terminal in VSCode IDE
   a) Command to execute in the terminal: python3 -m venv venv
   b) Activate virtual environment
   c) Command to execute in the terminal: source venv/bin/activate
   d) Install requirements from requirements.txt file
   e) Command to execute in the terminal: pip3 install -r requirements.txt
   f) Open the given Python code and adjust the COM port in the script to correspond with the COM port selected in the Arduino IDE
4. Make any changes in the Python code if necessary
5. Update port based on the Arduino port
6. Update baud rate based on the Arduino baud rate

**Setup Calibration**
1. Upload 'milk_impedance_sensor.ino' to the Arduino via the Arduino IDE.
2. Insert calibration impedance between J3 and J4 (usually a known resistor close to the unknown impedance, see PmodIA schematic)
3. Close Arduino Serial Monitor before running 'read_serial_data.py' to start calibration.
4. Run 'read_serial_data.py', follow serial printout.




