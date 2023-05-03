#pragma once

#include <sstream>
#include "DEFINITIONS.hpp"
#include "GameState.hpp"
#include "GameOverState.hpp"
#include "AIController.h"

#include <iostream>

#define REPLAY false

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
				//Initialize the bird with random gene data
				std::vector<std::vector<Node*>> _nodeNetwork = std::vector<std::vector<Node*>>();

				std::vector<Node*> inputNodes = std::vector<Node*>();
				for (int i = 0; i < 4; i++)
				{
					inputNodes.push_back(new InputNode());
				}
				_nodeNetwork.push_back(inputNodes);
				for (int i = 0; i < HIDDEN_LAYERS; i++)
				{
					std::vector<Node*> layer = std::vector<Node*>();
					for (int j = 0; j < NODES_PER_LAYER; j++)
					{
						layer.push_back(new ActivationNode());
					}
					_nodeNetwork.push_back(layer);
				}
				birds.push_back(new Bird(_data, _nodeNetwork));
			}
		}
		else
		{
			ImportBirds(_data, populationData);
			#if REPLAY == false
			Evolve(_data);
			generationNumber++;
			#endif
			//Reset the imported score to 0
			for (auto bird : birds)
			{
				bird->score = 0;
			}
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
						json nodeData;
						InputNode* node = static_cast<InputNode*>(birds.at(i)->nodeNetwork.at(j).at(k));
						nodeData["Weight"] = node->weight;
						layerData["Node" + std::to_string(k)] = nodeData;
					}
					geneData["InputLayer"] = layerData;
				}
				//Hidden Layers
				else
				{
					json layerData;
					//Iterate nodes
					for (int k = 0; k < birds.at(i)->nodeNetwork.at(j).size(); k++)
					{
						json nodeData;
						ActivationNode* node = static_cast<ActivationNode*>(birds.at(i)->nodeNetwork.at(j).at(k));
						
						nodeData["Weight"] = node->weight;
						nodeData["Bias"] = node->bias;
						layerData["Node" + std::to_string(k)] = nodeData;
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
	void GameState::ImportBirds(GameDataRef data, json populationData)
	{
		std::vector<Bird*> loadedBirds = std::vector<Bird*>();

		int geneIteration = 0;
		//iterate genes. Each iteration is a bird
		for (const auto& gene : populationData.items())
		{
			//Break if loading more than population count
			if (geneIteration >= POPULATION_SIZE)
				break;

			int score = 0;
			std::vector<std::vector<Node*>> nodeNetwork = std::vector<std::vector<Node*>>();

			int layerIteration = 0;
			for (const auto& geneData : gene.value().items())
			{
				if (geneData.key() == "Score")
					score = geneData.value();
				else if (geneData.key() == "InputLayer")
				{
					std::vector<Node*> layer = std::vector<Node*>();
					int nodeIteration = 0;
					for (const auto& layerData : geneData.value().items())
					{
						std::string key = layerData.key();
						if (layerData.key() == "Node" + std::to_string(nodeIteration))
						{
							float weight = 0;
							for (const auto& nodeData : layerData.value().items())
							{
								if (nodeData.key() == "Weight")
									weight = nodeData.value();
							}
							layer.push_back(new InputNode(weight));
						}
						nodeIteration++;
					}
					nodeNetwork.push_back(layer);
					layerIteration++;
				}
				else if (geneData.key() == "Layer" + std::to_string(layerIteration))
				{
					//Break if there are more layers being loaded
					if (layerIteration >= HIDDEN_LAYERS + 1)
						break;
					std::vector<Node*> layer = std::vector<Node*>();
					int nodeIteration = 0;
					for (const auto& layerData : geneData.value().items())
					{
						std::string key = layerData.key();
						//break if more nodes are being loaded
						if (nodeIteration >= NODES_PER_LAYER)
							break;
						if (layerData.key() == "Node" + std::to_string(nodeIteration))
						{
							float weight = 0;
							float bias = 0;
							for (const auto& nodeData : layerData.value().items())
							{
								std::string key = nodeData.key();
								if (nodeData.key() == "Weight")
									weight = nodeData.value();
								else if (nodeData.key() == "Bias")
									bias = nodeData.value();
							}
							layer.push_back(new ActivationNode(weight, bias));
						}
						nodeIteration++;
					}
					//Initialize remaining nodes, if there are less than specified
					for (int i = layer.size(); i < NODES_PER_LAYER; i++)
						layer.push_back(new ActivationNode());

					nodeNetwork.push_back(layer);
					layerIteration++;
				}
			}
			//Initialize remaining layers, if there are less than specified
			for (int i = nodeNetwork.size(); i < HIDDEN_LAYERS + 1; i++)
			{
				std::vector<Node*> layer = std::vector<Node*>();
				for (int j = 0; j < NODES_PER_LAYER; j++)
				{
					layer.push_back(new ActivationNode());
				}
				nodeNetwork.push_back(layer);
			}

			Bird* nextBird = new Bird(data, nodeNetwork);
			nextBird->score = score;
			loadedBirds.push_back(nextBird);
			geneIteration++;
		}
		//Initialize remaining birds to random, if loaded birds are less than pop size
		for (int i = loadedBirds.size(); i < POPULATION_SIZE; i++)
		{
			//Initialize the bird with random gene data
			std::vector<std::vector<Node*>> _nodeNetwork = std::vector<std::vector<Node*>>();

			std::vector<Node*> inputNodes = std::vector<Node*>();
			for (int i = 0; i < 4; i++)
			{
				inputNodes.push_back(new InputNode());
			}
			_nodeNetwork.push_back(inputNodes);
			for (int i = 0; i < HIDDEN_LAYERS; i++)
			{
				std::vector<Node*> layer = std::vector<Node*>();
				for (int j = 0; j < NODES_PER_LAYER; j++)
				{
					layer.push_back(new ActivationNode());
				}
				_nodeNetwork.push_back(layer);
			}
			loadedBirds.push_back(new Bird(data, _nodeNetwork));
		}

		birds = loadedBirds;
	}
	void GameState::Evolve(GameDataRef data)
	{
		//Initialize mating pool
		std::vector<Bird*> matingPool = std::vector<Bird*>();
		//Initialize output population
		std::vector<Bird*> output = std::vector<Bird*>();

		//Elitist selection
		output.push_back(birds.at(0));
		matingPool.push_back(birds.at(0));

		//Tournament selection
		while (matingPool.size() < MATING_POOL_SIZE)
		{
			//Select a couple birds at random, and pick the best one
			std::list<Bird*> selectionGroup = std::list<Bird*>();
			for (int i = 0; i < SELECTION_GROUP_SIZE; i++)
			{
				//Get random gene, add to selection group
				int randomIndex = rand() % birds.size();
				selectionGroup.push_back(birds.at(randomIndex));
			}
			selectionGroup.sort(Bird::BirdComparison);
			matingPool.push_back(*selectionGroup.begin());
		}
	

		//The mating pool has now been created, so perform crossover

		//Create offspring until output population is full
		while (output.size() < POPULATION_SIZE)
		{
			//Pick 2 parents from the input population
			Bird* parent1;
			Bird* parent2;
			int parent1Index = rand() % matingPool.size();
			parent1 = matingPool.at(parent1Index);

			//Ensuring that the second parent is not the same as first parent
			int parent2Index = parent1Index;
			while (parent1Index == parent2Index)
				parent2Index = rand() % matingPool.size();
			parent2 = matingPool.at(parent2Index);
			output.push_back(Crossover(data, parent1, parent2));
		}

		//Mutate

		//iterate output birds, excluding the elite
		for (int i = 1; i < output.size(); i++)
		{
			//iterate layers
			for (int j = 0; j < output.at(i)->nodeNetwork.size(); j++)
			{
				//Input layer
				if (j == 0)
				{
					//iterate nodes
					for (int k = 0; k < output.at(i)->nodeNetwork.at(j).size(); k++)
					{
						InputNode* node = static_cast<InputNode*>(output.at(i)->nodeNetwork.at(j).at(k));
						//Find if mutation happens
						int mutation = rand() % 101;
						if (mutation < MUTATION_RATE)
						{
							//positive or negative change
							int random = rand() % 2;
							float weight = node->weight;
							if (random == 0)
							{
								weight += MUTATION_ADJUSTMENT;
							}
							else
							{
								weight -= MUTATION_ADJUSTMENT;
							}
							node->weight = weight;
						}
					}
				}
				//Hidden layers
				else
				{
					//iterate nodes
					for (int k = 0; k < output.at(i)->nodeNetwork.at(j).size(); k++)
					{
						ActivationNode* node = static_cast<ActivationNode*>(output.at(i)->nodeNetwork.at(j).at(k));
						//Find if mutation happens on weight
						int mutation = rand() % 101;
						if (mutation < MUTATION_RATE)
						{
							//positive or negative change
							int random = rand() % 2;
							float weight = node->weight;
							if (random == 0) 
							{
								weight += MUTATION_ADJUSTMENT;
							}
							else 
							{
								weight -= MUTATION_ADJUSTMENT;
							}
							node->weight = weight;
						}
						//Find if mutation happens on bias
						mutation = rand() % 101;
						if (mutation < MUTATION_RATE)
						{
							//positive or negative change
							int random = rand() % 2;
							float bias = node->bias;
							if (random == 0)
							{
								bias += MUTATION_ADJUSTMENT;
							}
							else
							{
								bias -= MUTATION_ADJUSTMENT;
							}
							node->bias = bias;
						}
					}
				}
			}
		}


		//Delete old birds
		for (int i = 1; i < birds.size(); i++)
		{
			if (birds.at(i) != nullptr)
				delete birds.at(i);
		}
		birds.clear();

		birds = output;
	}
	Bird* GameState::Crossover(GameDataRef data, Bird* parent1, Bird* parent2)
	{
		std::vector<std::vector<Node*>> nodeNetwork = std::vector<std::vector<Node*>>();
		//iterate layers
		for (int i = 0; i < parent1->nodeNetwork.size(); i++)
		{
			//input layer
			if (i == 0)
			{
				std::vector<Node*> layer = std::vector<Node*>();
				//iterate nodes
				for (int j = 0; j < parent1->nodeNetwork.at(i).size(); j++)
				{
					InputNode* parent1Node = static_cast<InputNode*>(parent1->nodeNetwork.at(i).at(j));
					InputNode* parent2Node = static_cast<InputNode*>(parent2->nodeNetwork.at(i).at(j));
					float difference = 0;
					//find if the numbers have same sign, then find difference
					if (std::signbit(parent1Node->weight) == std::signbit(parent2Node->weight))
					{
						if (abs(parent1Node->weight) > abs(parent2Node->weight))
							difference = abs(parent1Node->weight) - abs(parent2Node->weight);
						else
							difference = abs(parent2Node->weight) - abs(parent1Node->weight);
					}
					else
						difference = abs(parent1Node->weight) + abs(parent2Node->weight);
					float adjustment = difference * CROSSOVER_RATE;
					int random = rand() % 2;
					float weight = 0;
					//Choose a weight from the 2 parents, and adjust it towards the other parent
					if (random == 0) 
					{
						weight = parent1Node->weight;
						if (parent1Node->weight > parent2Node->weight)
							weight -= adjustment;
						else
							weight += adjustment;
					}
					else 
					{
						weight = parent2Node->weight;
						if (parent2Node->weight > parent1Node->weight)
							weight -= adjustment;
						else
							weight += adjustment;
					}
					layer.push_back(new InputNode(weight));

				}
				nodeNetwork.push_back(layer);
			}
			//Hidden layers
			else 
			{
				std::vector<Node*> layer = std::vector<Node*>();
				//iterate nodes
				for (int j = 0; j < parent1->nodeNetwork.at(i).size(); j++)
				{
					ActivationNode* parent1Node = static_cast<ActivationNode*>(parent1->nodeNetwork.at(i).at(j));
					ActivationNode* parent2Node = static_cast<ActivationNode*>(parent2->nodeNetwork.at(i).at(j));
					float weightDifference = 0;
					//find if the numbers have same sign, then find difference
					if (std::signbit(parent1Node->weight) == std::signbit(parent2Node->weight))
					{
						if (abs(parent1Node->weight) > abs(parent2Node->weight))
							weightDifference = abs(parent1Node->weight) - abs(parent2Node->weight);
						else
							weightDifference = abs(parent2Node->weight) - abs(parent1Node->weight);
					}
					else
						weightDifference = abs(parent1Node->weight) + abs(parent2Node->weight);
					float adjustment = weightDifference * CROSSOVER_RATE;
					int random = rand() % 2;
					float weight = 0;
					//Choose a weight from the 2 parents, and adjust it towards the other parent
					if (random == 0)
					{
						weight = parent1Node->weight;
						if (parent1Node->weight > parent2Node->weight)
							weight -= adjustment;
						else
							weight += adjustment;
					}
					else
					{
						weight = parent2Node->weight;
						if (parent2Node->weight > parent1Node->weight)
							weight -= adjustment;
						else
							weight += adjustment;
					}

					float biasDifference = 0;
					//find if the numbers have same sign, then find difference
					if (std::signbit(parent1Node->bias) == std::signbit(parent2Node->bias))
					{
						if (abs(parent1Node->bias) > abs(parent2Node->bias))
							weightDifference = abs(parent1Node->bias) - abs(parent2Node->bias);
						else
							weightDifference = abs(parent2Node->bias) - abs(parent1Node->bias);
					}
					else
						weightDifference = abs(parent1Node->bias) + abs(parent2Node->bias);
					adjustment = biasDifference * CROSSOVER_RATE;
					random = rand() % 2;
					float bias = 0;
					//Choose a bias from the 2 parents, and adjust it towards the other parent
					if (random == 0)
					{
						bias = parent1Node->bias;
						if (parent1Node->bias > parent2Node->bias)
							bias -= adjustment;
						else
							bias += adjustment;
					}
					else
					{
						bias = parent2Node->bias;
						if (parent2Node->bias > parent1Node->bias)
							bias -= adjustment;
						else
							bias += adjustment;
					}

					layer.push_back(new ActivationNode(weight, bias));
				}
				nodeNetwork.push_back(layer);
			}
		}
		return new Bird(data, nodeNetwork);
	}
}