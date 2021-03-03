/*
* Player.h
*
* MIT License
*
* Copyright (c) 2021 Frank Hale
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

class Player
{
public:
		Player(int starting_health, int starting_attack);

		int GetStartingHealth() const { return starting_health; }
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

