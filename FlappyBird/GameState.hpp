#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <string>

#include "State.hpp"
#include "Game.hpp"
#include "Pipe.hpp"
#include "Land.hpp"
#include "Bird.hpp"
#include "Collision.hpp"
#include "Flash.hpp"
#include "HUD.hpp"

//library for json files, namepspace definition
#include "nlohmann/json.hpp"
using json = nlohmann::json;

class AIController;

namespace Sonar
{
	class GameState : public State
	{
	public:
		GameState(GameDataRef data);
		~GameState() override;

		void Init();

		void HandleInput();
		void Update(float dt);
		void Draw(float dt);

		Pipe* GetPipeContainer() { return pipe; };
		Land* GetLandContainer() { return land; };
		std::vector<Bird*> GetBirds() { return birds; }

	private:
		//Saves the bird list to a json file
		void ExportBirds();
		//Imports the bird list from a json file
		void ImportBirds(json populationData);
		
		//Evolves the bird list and creates the next generation
		void Evolve();

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

		int generationNumber = -1;

		sf::SoundBuffer _hitSoundBuffer;
		sf::SoundBuffer _wingSoundBuffer;
		sf::SoundBuffer _pointSoundBuffer;

		sf::Sound _hitSound;
		sf::Sound _wingSound;
		sf::Sound _pointSound;

		AIController* m_pAIController;

		bool initialized = false;
	};
}