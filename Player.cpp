#include "Player.h"

Player::Player(int starting_health, int starting_attack)
{
		x = 10;
		y = 10;
		score = 0;
		enemies_killed = 0;
		this->starting_health = starting_health;
		health = starting_health;
		attack = starting_attack;
}