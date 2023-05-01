#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "State.hpp"
#include "Game.hpp"
#include "Pipe.hpp"
#include "Land.hpp"
#include "Bird.hpp"
#include "Collision.hpp"
#include "Flash.hpp"
#include "HUD.hpp"

class AIController;

namespace Sonar
{
	class GameState : public State
	{
	public:
		GameState(GameDataRef data);
		~GameState();

		void Init();

		void HandleInput();
		void Update(float dt);
		void Draw(float dt);

		Pipe* GetPipeContainer() { return pipe; };
		Land* GetLandContainer() { return land; };
		std::vector<Bird*> GetBirds() { return birds; }

	private:
		GameDataRef _data;

		sf::Sprite _background;

		Pipe *pipe;
		Land *land;
		std::vector<Bird*> birds;
		//Bird *bird;
		Collision collision;
		Flash *flash;
		HUD *hud;

		sf::Clock clock;

		int _gameState;

		sf::RectangleShape _gameOverFlash;
		bool _flashOn;

		int _score;

		sf::SoundBuffer _hitSoundBuffer;
		sf::SoundBuffer _wingSoundBuffer;
		sf::SoundBuffer _pointSoundBuffer;

		sf::Sound _hitSound;
		sf::Sound _wingSound;
		sf::Sound _pointSound;

		AIController* m_pAIController;
	};
}