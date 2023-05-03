#pragma once

#include <sstream>
#include "DEFINITIONS.hpp"
#include "GameState.hpp"
#include "GameOverState.hpp"
#include "AIController.h"

#include <iostream>

#define REPLAY true

#define PLAY_WITH_AI 1

namespace Sonar
{
	GameState::GameState(GameDataRef data) : _data(data)
	{
		initialized = false;
		m_pAIController = new AIController();
		m_pAIController->setGameState(this);
	}

	GameState::~GameState()
	{
		if(pipe != nullptr)
			delete pipe;
		if (land != nullptr)
			delete land;
		if (flash != nullptr)
			delete flash;
		if (hud != nullptr)
			delete hud;
		for (auto bird : birds)
		{
			if(bird != nullptr)
				delete bird;
		}
		birds.clear();
	}



	void GameState::Init()
	{
		if (!_hitSoundBuffer.loadFromFile(HIT_SOUND_FILEPATH))
		{
			std::cout << "Error Loading Hit Sound Effect" << std::endl;
		}

		if (!_wingSoundBuffer.loadFromFile(WING_SOUND_FILEPATH))
		{
			std::cout << "Error Loading Wing Sound Effect" << std::endl;
		}

		if (!_pointSoundBuffer.loadFromFile(POINT_SOUND_FILEPATH))
		{
			std::cout << "Error Loading Point Sound Effect" << std::endl;
		}

		_hitSound.setBuffer(_hitSoundBuffer);
		_wingSound.setBuffer(_wingSoundBuffer);
		_pointSound.setBuffer(_pointSoundBuffer);

		this->_data->assets.LoadTexture("Game Background", GAME_BACKGROUND_FILEPATH);
		this->_data->assets.LoadTexture("Pipe Up", PIPE_UP_FILEPATH);
		this->_data->assets.LoadTexture("Pipe Down", PIPE_DOWN_FILEPATH);
		this->_data->assets.LoadTexture("Land", LAND_FILEPATH);
		this->_data->assets.LoadTexture("Bird Frame 1", BIRD_FRAME_1_FILEPATH);
		this->_data->assets.LoadTexture("Bird Frame 2", BIRD_FRAME_2_FILEPATH);
		this->_data->assets.LoadTexture("Bird Frame 3", BIRD_FRAME_3_FILEPATH);
		this->_data->assets.LoadTexture("Bird Frame 4", BIRD_FRAME_4_FILEPATH);
		this->_data->assets.LoadTexture("Scoring Pipe", SCORING_PIPE_FILEPATH);
		this->_data->assets.LoadFont("Flappy Font", FLAPPY_FONT_FILEPATH);

		pipe = new Pipe(_data);
		land = new Land(_data);

		std::string epochDirectory = "epochs/";
		json populationData;
		generationNumber = -1;

		while (true)
		{
			std::ifstream epochFile(epochDirectory + "epoch" + std::to_string(generationNumber + 1) + ".json");
			if (!epochFile.good())
				break; //File doesn't exist, end here
			populationData = json::parse(epochFile);
			generationNumber++;
		}
		//If this is the first generation, create pop0
		if (generationNumber == -1)
		{
			generationNumber++;
			for (int i = 0; i < POPULATION_SIZE; i++)
			{
				//Initialize the genetic data of the bird to a network of nodes
				std::vector<std::vector<Node*>> nodeNetwork = std::vector<std::vector<Node*>>();

				std::vector<Node*> inputNodes = std::vector<Node*>();
				for (int i = 0; i < 4; i++)
				{
					inputNodes.push_back(new InputNode());
				}
				nodeNetwork.push_back(inputNodes);
				for (int i = 0; i < HIDDEN_LAYERS; i++)
				{
					std::vector<Node*> layer = std::vector<Node*>();
					for (int j = 0; j < NODES_PER_LAYER; j++)
					{
						layer.push_back(new ActivationNode());
					}
					nodeNetwork.push_back(layer);
				}

				birds.push_back(new Bird(_data, nodeNetwork));
			}
		}
		else
		{
			ImportBirds(populationData);
			#if REPLAY == false
			Evolve();
			generationNumber++;
			#endif
		}


		flash = new Flash(_data);
		hud = new HUD(_data);

		_background.setTexture(this->_data->assets.GetTexture("Game Background"));

		_score = 0;
		hud->UpdateScore(_score);

		initialized = true;
		_gameState = GameStates::eReady;
	}

	void GameState::HandleInput()
	{
#if PLAY_WITH_AI
		if (GameStates::eGameOver != _gameState)
		{
			_gameState = GameStates::ePlaying;

			for (auto bird : birds) {
				if (bird->isAlive) {
					m_pAIController->update(bird);

					if (m_pAIController->shouldFlap())
					{
						std::cout << "tap!" << std::endl;
						bird->Tap();
						//_wingSound.play();
					}
				}
			}
		}

#endif

		sf::Event event;
		while (this->_data->window.pollEvent(event))
		{
			if (sf::Event::Closed == event.type)
			{
				this->_data->window.close();
			}

			if (this->_data->input.IsSpriteClicked(this->_background, sf::Mouse::Left, this->_data->window))
			{
				if (GameStates::eGameOver != _gameState)
				{
					std::cout << "tap!" << std::endl;
					_gameState = GameStates::ePlaying;
					int rand = std::rand() % birds.size();
					while(!birds.at(rand)->isAlive)
						rand = std::rand() % birds.size();

					birds.at(rand)->Tap();

					//bird->Tap();

					//_wingSound.play();
				}
			}
		}
	}

	void GameState::Update(float dt)
	{
		if (GameStates::eGameOver != _gameState)
		{
			for (auto bird : birds) {
				if(bird->isAlive)
					bird->Animate(dt);
			}
			land->MoveLand(dt);
		}

		if (GameStates::ePlaying == _gameState)
		{
			pipe->MovePipes(dt);

			if (clock.getElapsedTime().asSeconds() > (PIPE_SPAWN_FREQUENCY/GAME_SPEED))
			{
				pipe->RandomisePipeOffset();

				pipe->SpawnInvisiblePipe();
				pipe->SpawnBottomPipe();
				pipe->SpawnTopPipe();
				pipe->SpawnScoringPipe();

				clock.restart();
			}
			for (auto bird : birds) {
				bird->Update(dt);
			}

			std::vector<sf::Sprite> landSprites = land->GetSprites();

			for (unsigned int i = 0; i < landSprites.size(); i++)
			{
				for (auto bird : birds) {
					if (bird->isAlive) {
						if (collision.CheckSpriteCollision(bird->GetSprite(), 0.7f, landSprites.at(i), 1.0f, false))
						{
							bird->isAlive = false;
							std::cout << "death" << std::endl;
							//_gameState = GameStates::eGameOver;

							//clock.restart();

							//_hitSound.play();
						}
					}
				}
			}

			std::vector<sf::Sprite> pipeSprites = pipe->GetSprites();

			for (unsigned int i = 0; i < pipeSprites.size(); i++)
			{
				for (auto bird : birds) {
					if (bird->isAlive) {
						if (collision.CheckSpriteCollision(bird->GetSprite(), 0.625f, pipeSprites.at(i), 1.0f, true))
						{
							bird->isAlive = false;
							//_gameState = GameStates::eGameOver;
							std::cout << "death" << std::endl;
							//clock.restart();

							//_hitSound.play();
						}
					}
				}
			}
			//Find game over
			if (initialized) {
				bool dead = true;
				for (auto bird : birds)
				{
					if (bird->isAlive)
						dead = false;
				}
				if (dead)
				{
					if (generationNumber == 0)
					{
						ExportBirds();
					}
					else
					{
						#if REPLAY == false
						ExportBirds();
						#endif
					}
					_gameState = GameStates::eGameOver;
					clock.restart();
				}
			}

			if (GameStates::ePlaying == _gameState)
			{
				std::vector<sf::Sprite> &scoringSprites = pipe->GetScoringSprites();

				for (unsigned int i = 0; i < scoringSprites.size(); i++)
				{
					for (auto bird : birds) {
						if (bird->isAlive) {
							if (collision.CheckSpriteCollision(bird->GetSprite(), 0.625f, scoringSprites.at(i), 1.0f, false))
							{
								_score++;

								hud->UpdateScore(_score);

								scoringSprites.erase(scoringSprites.begin() + i);

								//_pointSound.play();
							}
						}
					}
				}
				for (auto bird : birds)
				{
					if (bird->isAlive)
						bird->score = _score;
				}
			}
		}

		if (GameStates::eGameOver == _gameState)
		{
			flash->Show(dt);

			if (clock.getElapsedTime().asSeconds() > TIME_BEFORE_GAME_OVER_APPEARS)
			{
				this->_data->machine.AddState(new GameOverState(_data, _score), true);
			}
		}
	}

	void GameState::Draw(float dt)
	{
		this->_data->window.clear( sf::Color::Red );

		this->_data->window.draw(this->_background);

		pipe->DrawPipes();
		land->DrawLand();
		for (auto bird : birds) {
			if (bird->isAlive) {
				bird->Draw();
			}
		}

		flash->Draw();

		hud->Draw();

		this->_data->window.display();
	}
	void GameState::ExportBirds()
	{
		std::list<Bird*> populationAsList = std::list<Bird*>(birds.begin(), birds.end());
		populationAsList.sort(Bird::BirdComparison);
		birds = std::vector<Bird*>(populationAsList.begin(), populationAsList.end());

		std::string epochDirectory = "epochs/";
		json populationData;

		//iterate birds
		for (int i = 0; i < birds.size(); i++) 
		{
			json geneData;
			geneData["Score"] = birds.at(i)->score;
			//Iterate layers
			for (int j = 0; j < birds.at(i)->nodeNetwork.size(); j++)
			{
				json layerData;
				//Input Layer
				if (j == 0)
				{
					//iterate nodes
					for (int k = 0; k < birds.at(i)->nodeNetwork.at(j).size(); k++)
					{
						InputNode* node = static_cast<InputNode*>(birds.at(i)->nodeNetwork.at(j).at(k));
						layerData["Weights"].push_back(node->weight);
					}
					geneData["InputLayer"].push_back(layerData);
				}
				//Hidden Layers
				else
				{
					json layerData;
					//Iterate nodes
					for (int k = 0; k < birds.at(i)->nodeNetwork.at(j).size(); k++)
					{
						ActivationNode* node = static_cast<ActivationNode*>(birds.at(i)->nodeNetwork.at(j).at(k));
						
						layerData["Weights"].push_back(node->weight);
						layerData["Biases"].push_back(node->bias);
					}
					geneData["Layer" + std::to_string(j)] = layerData;
				}
			}
			populationData["Gene" + std::to_string(i + 1)] = geneData;
		}
		//Write the json data to the file
		std::ofstream outputFile(epochDirectory + "epoch" + std::to_string(generationNumber) + ".json");
		if (!outputFile.good())
			return;
		outputFile << std::setw(4) << populationData;
	}
	void GameState::ImportBirds(json populationData)
	{
	}
	void GameState::Evolve()
	{
	}
}