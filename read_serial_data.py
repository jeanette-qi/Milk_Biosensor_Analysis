"""
This script reads data sent from the Arduino IDE via a serial port,
parses it, and saves the results to an Excel file in a specified Google Drive folder.
Update the path and port if needed.
"""
import os
import time
from datetime import datetime
import serial
import pandas as pd
import numpy as np
from scipy.signal import iirnotch, filtfilt
from serial.tools import list_ports

def serial_connection():
    """
    This function sets up a serial connection, reads data,
    and saves it to an Excel file.
    """
    # Set up the serial connection
    port = '/dev/cu.usbmodem101'  # The port your Arduino is connected to
    baudrate = 9600  # The baud rate set in your Arduino sketch
    timeout = 5  # Timeout for reading from the serial port

    try:
        ser = serial.Serial(port, baudrate, timeout=timeout)
        print(f"Connected to {port} at {baudrate} baud.")
        return ser

    except serial.SerialException as e:
        print(f"Error: {e}")
        return None


def create(csv_files):
    """
    This function checks to see if the specified folder exists.
    """
       # Ensure the folder exists
    if not os.path.exists(csv_files):
        os.makedirs(csv_files, exist_ok=True)
    else:
        print(f'Folder already exists: {csv_files}')

def notch_filter(impedance, fs=10000, notch_freq=2000, Q=30):
    """
    Applies a notch filter to remove 2kHz noise.
    """
    try:
        b, a = iirnotch(notch_freq, Q, fs)
        return filtfilt(b, a, impedance)
    except Exception as e:
        print(f"Error applying notch filter: {e}")
        return impedance  # Return unfiltered data on failure

def process(ser, csv_files):
    """
    This function parses, reads, filters, and formats the data from the serial port.
    """
   # ser = serial_connection() #ser variable from prev function

    try:
        # Wait for the serial connection to initialize
        time.sleep(2)

        #Add the test impedance and wait for the Arduino prompt before sending 'y'
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                print(f"Arduino: {line}")
                
                if "press 'y'" in line.lower():
                    while True:
                        user_input = input("Type 'y' and press Enter to continue: ").strip().lower()
                        if user_input == 'y':
                            ser.write(b'y\n')
                            ser.flush()
                            print("Sent 'y' to Arduino.")
                            break
                        else:
                            print("Invalid input. Please type 'y'.")
                    break  # break outer loop to proceed

    
        print("Connected to Arduino")

        data_digest = []

        while True:
            if ser.in_waiting > 0:
                # Read a line from the serial port
                line = ser.readline()
                decoded_output = line.decode('utf-8', errors='replace').strip()
                print(f"Received: {decoded_output}")

                if decoded_output.startswith('-'):  # Skip invalid lines
                    continue

                # Skip non-data lines
                # Adjust as per your data format
                if not decoded_output.startswith("1,frequency_sweep_easy"):
                    continue

                # Parse the data
                res = decoded_output.split(',')
                res.insert(0, datetime.now().isoformat())  # Add a timestamp
                data_digest.append(res)

    except KeyboardInterrupt:
        # Close the serial connection on a keyboard interrupt
        print("\nKeyboard Interrupt detected. Closing connection.")

    finally:
        # Ensure serial connection is properly closed
        if 'ser' and ser.is_open:
            ser.close()
            print("Serial connection closed.")

        if data_digest:
            # Returns a string representing date and time
            now_time = datetime.now().strftime("%Y%m%d_%H%M%S")
            new_filename = f'bioimpedance{now_time}.xlsx'  # CSV filename with date and time
            file_path = os.path.join(csv_files, new_filename)  # Path to Google Drive

            try:
                df = pd.DataFrame(data_digest, columns=[
                    'timestamp', 'iteration_no', 'frequency_sweep_type',
                    'c_frequency', 'real', 'imag', 'gain', 'magnitude', 'impedance', 'phase'
                ])
                # Convert impedance to float before filtering
                df['impedance'] = df['impedance'].astype(float)
                #df['impedance_filtered'] = notch_filter(df['impedance'].to_numpy())

                df.to_excel(file_path, index=False, header=True)
                print(f'Data saved to Google Drive: {file_path} successfully')
                return file_path
            
            except Exception as e:
                print(f"Error saving file: {e}")

        else:
            print("No data to save.")
            return None


def main():
    """
    This is the main function to manage the setup, establishing a 
    serial connection, and data processing.
    """
    # Path to save data as csv file to Google Drive folder, update as needed.
    csv_files = (
        '/Users/jeanetteqi/Library/CloudStorage/GoogleDrive-jeanette.qi@sjsu.edu/'
        'Shared drives/MS project_Bryant and Sejad_Human Milk'
        ' for Premature Infants/Prototype_MilkSensor_CSVData'
    )
    csv_folder = os.path.join(csv_files, "Prototype_MilkSensor_CSVData")

    create(csv_folder)
    ser = serial_connection()
        
    if ser:
        process(ser, csv_folder)
    else:
        print("Failed to establish a serial connection.")


if __name__ == "__main__":
    main()
