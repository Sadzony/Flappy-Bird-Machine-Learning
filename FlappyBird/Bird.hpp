#pragma once

#include <SFML/Graphics.hpp>

#include "DEFINITIONS.hpp"
#include "Game.hpp"
#include "Node.h"

#include <vector>

namespace Sonar
{
	class Bird
	{
	public:
		Bird(GameDataRef data, std::vector<std::vector<Node*>> p_nodeNetwork);
		~Bird();

		void Draw();

		void Animate(float dt);

		void Update(float dt);

		void Tap();

		const sf::Sprite &GetSprite() const;

		void getHeight(int& x, int& y);

		int score = 0;
		bool isAlive = true;

	private:
		GameDataRef _data;

		sf::Sprite _birdSprite;
		std::vector<sf::Texture> _animationFrames;

		unsigned int _animationIterator;

		sf::Clock _clock;

		sf::Clock _movementClock;

		int _birdState;

		float _rotation;

		std::vector<std::vector<Node*>> nodeNetwork;
	};
}