#pragma once
class Player
{
public:
		Player(int starting_health, int starting_attack);

		int GetPlayerStartingHealth() const { return starting_health; }
		int GetHealth() const { return health; }
		int GetScore() const { return score; }
		int GetAttack() const { return attack; }
		int GetEnemiesKilled() const { return enemies_killed; }

		void SetHealth(int h) { health = h; }
		void SetScore(int s) { score = s; }
		void SetAttack(int a) { attack = a; }
		void SetEnemiesKilled(int k) { enemies_killed = k; }

		int X() const { return x; }
		int Y() const { return y; }
		void X(int _x) { x = _x; }
		void Y(int _y) { y = _y; }

private:

		int starting_health, health, score, attack, enemies_killed;
		int x, y;
};

