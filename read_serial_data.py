import os
import serial
import time
import pandas as pd
from serial.tools import list_ports
from datetime import datetime



# Set up the serial connection
port = '/dev/cu.usbmodem101'  # The port your Arduino is connected to
baudrate = 9600  # The baud rate set in your Arduino sketch
timeout = 5  # Timeout for reading from the serial port


ser = serial.Serial(port, baudrate, timeout=timeout)
#print(f"Is serial open? {ser.is_open}")

#Add new csv file to Google Drive folder, update path if needed
csv_files = '/Users/jeanetteqi/Library/CloudStorage/GoogleDrive-jeanette.qi@sjsu.edu/Shared drives/MS project_Bryant and Sejad_Human Milk for Premature Infants/Prototype_MilkSensor_CSVData'

try:
    # Wait for the serial connection to initialize
    time.sleep(2)
    print("Connected to Arduino")

    data_digest = []

    while True:
        if ser.in_waiting > 0:
            #Read a line from the serial port
            line = ser.readline()
            decoded_output = line.decode().rstrip()
            print(f"Received: {decoded_output}")

            
            #if decoded_output.startswith('-'):
                #continue

            # Handle both bytes and string cases
            if isinstance(line, bytes):
                line = line.decode()
            line = line.strip()  # Remove leading/trailing whitespace
            print(f"Received: {line}")

            # Skip non-data lines
            if not line.startswith("1,frequency_sweep_easy"):  # Adjust as per your data format
                continue
            # Parse the data
            res = line.split(',')
            res.insert(0, datetime.now().isoformat())  # Add a timestamp
            data_digest.append(res)
        

except KeyboardInterrupt:
    # Close the serial connection on a keyboard interrupt
    print("Closing connection")
    ser.close()

finally:
    if data_digest:
        # Ensure the Google Drive folder exists
        if not os.path.exists(csv_files):
            os.makedirs(csv_files, exist_ok=True)

        now_time= datetime.now().strftime("%Y%m%d_%H%M%S") #Returns a string representing date and time
        new_filename = f'bioimpedance{now_time}.xlsx' #CSV filename with date and time

        file_path = os.path.join(csv_files, new_filename) #path to Google Drive
        
        try:
            df = pd.DataFrame(data_digest, columns=['timestamp','iteration_no', 'frequency_sweep_type', 'c_frequency', 'real', 'imag', 'gain', 'magnitude', 'impedance'])
            df.to_excel(file_path, index=False, header=True)
            print(f'Data saved to Google Drive: {file_path} successfully')
        except Exception as e:
            print(f"Error saving file: {e}")
    else:
         print("No data to save.")


