// Load stream
#include <iostream>
#include "Player.h"

// Set namespace
using namespace std;

// Main program
int main()
{
	// Create a character as a local variable on the stack using the default constructor
	Player deeFalt;

	// Create another character on the stack using the parameterized constructor
	Player rorsttDedragon("Rorstt Dedragon", 20, 15, 16, 14);

	// Create 1 more character on the heap using the parameterized constructor
	Player* charrdGaval = new Player("Charrd Gaval", 15, 18, 16, 12);

	// Now print all of the Player characters out (NOTE: Calling methods of objects referred to by pointers requires the use of the arrow operator)
	deeFalt.printPlayer();
	rorsttDedragon.printPlayer();
	charrdGaval->printPlayer();

	// Delete all the objects that were created using "new" 
	delete charrdGaval;
	charrdGaval = NULL;

	return 0;
}