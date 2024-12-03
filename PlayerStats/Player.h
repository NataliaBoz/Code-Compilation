#pragma once
class Player
{
public:
	Player();
	// Parameterized Constructor
	Player(const char* playerName, int strength, int dexterity, int constitution, int charisma);
	void printPlayer();

private: 
	const char* playerName; // Must be a const in order to store string literals ""
	int strength;
	int dexterity;
	int constitution;
	int charisma;
};

