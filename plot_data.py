import pandas as pd
import matplotlib.pyplot as plt
import os

def plot():
    
    
    #df = pd.read_excel('bioimpedance.xlsx', header=0)
    #freq_50_data = df[df['c_frequency'] == 50]
    #print(df.head(5))
    #print(freq_50_data['impedance'])

    user_input = input("Enter the name or description to include in the plot title (e.g., 'Milk Sensor'): ")

    
    # Add Google Drive directory path
    google_drive_path = '/Users/jeanetteqi/Library/CloudStorage/GoogleDrive-jeanette.qi@sjsu.edu/Shared drives/MS project_Bryant and Sejad_Human Milk for Premature Infants/Prototype_MilkSensor_CSVData'
    
    try:
        #Check if path exists
        if not os.path.exists(google_drive_path):
            print(f'No Excel files found in the specified {google_drive_path}')
            return
       
        #List all Excel files in folder
        file_registry = [file for file in os.listdir(google_drive_path) if file.endswith('.xlsx')]
        
        #If no Excel files
        if not file_registry:
            print("No Excel files found in the specified folder.")
            return
       
        # Find the most recently modified Excel file
        recent_file = max(
            [os.path.join(google_drive_path, file) for file in file_registry],
            key=os.path.getctime
        )
        print(f"Loading data from: {recent_file}")
        
        # Load the data from the most recent file
        df = pd.read_excel(recent_file, header=0)

    except Exception as e:
        print(f"An error occurred: {e}")
        return
    
    plt.figure(figsize=(10, 5))
    plt.plot(df['c_frequency']*1000, df['impedance'], marker='o', linestyle='-', color='b', label='Impedance vs Frequency')
  

    # Adding titles and labels
    plt.title('Frequency vs Impedance'+' '+ 'of' +' ' + user_input)
    plt.xlabel('Frequency' + ' (Hz)')
    plt.ylabel('Impedance' + ' (ohms)')

    plt.legend()

    # Rotating the x-axis labels for better readability
    plt.xticks(rotation=45)

    # Display the plot
    plt.tight_layout()  # Adjust layout to prevent clipping of tick-labels
    
   
    #Saves plots with filename, without the path
    #Splitext removes the xlsx, [0] keeps first part of text
    #https://docs.python.org/3/library/os.path.html

    filefile = os.path.splitext(os.path.basename(recent_file))[0] 
    save_plot = os.path.join(google_drive_path, f'{filefile}_{user_input}_plot.pdf')
    plt.savefig(save_plot, format='pdf')  # Save the plot as PDF
    print(f"Plot saved at: {save_plot}")
    
    plt.autoscale() #scale according to data
    plt.show() #Need to move below plt.savefig or it clears before saving
  
if __name__ == '__main__':
    plot()