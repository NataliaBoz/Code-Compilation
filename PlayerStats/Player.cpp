#include "Player.h"
#include <iostream>
using namespace std;

// Default Constructor
Player::Player()
{
	// Set a string literal to point to
	playerName = "Unknown";
	// Set all other int values to start at 10
	strength = 10;
	dexterity = 10;
	constitution = 10;
	charisma = 10;
}

// Parameterized Constructor
Player::Player(const char* playerName, int strength, int dexterity, int constitution, int charisma)
{
	this->playerName = playerName; // Must be a const in order to store string literals ""
	this->strength = strength;
	this->dexterity = dexterity;
	this->constitution = constitution;
	this->charisma = charisma;
}

// Print out the player�s name & stats 1 per line
void Player::printPlayer()
{
	cout << "Name: " << playerName << "\nStrength: " << strength << "\nDexterity: " 
		<< dexterity << "\nConstitution: " << constitution << "\nCharisma: " << charisma << endl << endl; 
}