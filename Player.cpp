#include "Player.h"

Player::Player(int starting_health, int starting_attack)
{
		x = 1;
		y = 1;
		score = 0;
		enemies_killed = 0;
		health = starting_health;		
		attack = starting_attack;		
}