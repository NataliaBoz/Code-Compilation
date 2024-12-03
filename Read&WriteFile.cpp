/*
* Natalia Boz.
*/ 

#include <iostream>
// Load stream for File IO
#include <fstream>
// Load for string data type & getline(,)
#include <string>

using namespace std;

int main()
{
    // Use an ofstream object to create & write a few lines to a text file
    ofstream writeFile;
    // Create/open a file with this name
    writeFile.open("RW-File.txt");

    // Check that the file was found opened before attempting to use it
    if (writeFile.is_open())
    {
        // Write some text to the file
        writeFile << "Never Gonna Give You Up: \nWe're no strangers to love\n ";
        writeFile << "You know the rules and so do I... (;";
        // Close the file
        writeFile.close();
    }
    else
    {
        // Inform the user if there was an error opening the file
        cout << "ERROR Unable to open file\n";
    }

    // Use an ifstream object to read the file in ios::binary mode
    ifstream readFile("RW-File.txt", ios::binary);
    string line;
    string text; 

    // Read the entire file into a std::string
    if (readFile.is_open())
    {
        while (getline(readFile, line))
        {
            text += line; 
        }
        readFile.close(); 
    }
    
    // Print the file's contents to the console window
    cout << text << endl; 

    return 0;
}
