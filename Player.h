#pragma once
class Player
{
public:
    Player(int starting_health, int starting_attack);    

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

    void SetX(int _x) { x = _x; }
    void SetY(int _y) { y = _y; }    

private:

    int health, score, attack, enemies_killed;
    int x, y;
};

