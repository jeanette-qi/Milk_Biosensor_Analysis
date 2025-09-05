import os
import pandas as pd
import matplotlib.pyplot as plt

def load_latest_data(directory):
    """
    Loads the most recent CSV file from the directory.
    """
    try:
        files = [f for f in os.listdir(directory) if f.endswith('.xlsx')]

        if not files:
            print("No CSV files found.")
            return None, None

        latest_file = max([os.path.join(directory, f) for f in files], key=os.path.getctime)
        print(f"Loading data from: {latest_file}")
        return pd.read_excel(latest_file), latest_file

    except Exception as e:
        print(f"Error loading data: {e}")
        return None, None

def plot(df, user_input, save_folder, file_name):
    """
    Plots impedance vs frequency and saves the plot.
    """
    if 'c_frequency' not in df.columns or 'impedance' not in df.columns:
        print("Required columns not found in dataset.")
        return

    plt.figure(figsize=(10, 5))
    plt.plot(df['c_frequency'] * 1000, df['impedance'], marker='o', linestyle='-', alpha=0.6, label='Original Impedance')
    #plt.plot(df['c_frequency'] * 1000, df['impedance_filtered'], marker='o', linestyle='-', color='r', label='Filtered Impedance')

    plt.title(f'Frequency vs Impedance of {user_input}')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Impedance (ohms)')
    plt.legend()
    plt.xticks(rotation=45)
    plt.tight_layout()

    # Ensure the folder exists before saving
    if not os.path.exists(save_folder):
        os.makedirs(save_folder, exist_ok=True)

    plot_filename = os.path.join(save_folder, f"{file_name}_{user_input}_plot.pdf")
    plt.savefig(plot_filename, format='pdf')
    print(f"Plot saved at: {plot_filename}")

    plt.show()

def main():
    """
    Main function to load the latest data and generate a plot.
    """
    GOOGLE_DRIVE_PATH = "/Users/jeanetteqi/Library/CloudStorage/GoogleDrive-jeanette.qi@sjsu.edu/Shared drives/MS project_Bryant and Sejad_Human Milk for Premature Infants/Prototype_MilkSensor_CSVData"
    CSV_FOLDER = os.path.join(GOOGLE_DRIVE_PATH, "Prototype_MilkSensor_CSVData")
    PLOT_FOLDER = os.path.join(GOOGLE_DRIVE_PATH, "Plots")

    df, latest_file = load_latest_data(CSV_FOLDER)

    if df is not None:
        #Ask for user input and pass it to `plot()`
        user_input = input("Enter the name or description to include in the plot title (e.g., 'Milk Sensor'): ")
        plot(df, user_input, PLOT_FOLDER, os.path.splitext(os.path.basename(latest_file))[0])

if __name__ == '__main__':
    main()
