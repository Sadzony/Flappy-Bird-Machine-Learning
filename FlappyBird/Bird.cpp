#include "Bird.hpp"

namespace Sonar
{
	Bird::Bird(GameDataRef data, std::vector<std::vector<Node*>> p_nodeNetwork) : _data(data)
	{
		nodeNetwork = std::vector<std::vector<Node*>>(p_nodeNetwork);

		_animationIterator = 0;

		_animationFrames.push_back(this->_data->assets.GetTexture("Bird Frame 1"));
		_animationFrames.push_back(this->_data->assets.GetTexture("Bird Frame 2"));
		_animationFrames.push_back(this->_data->assets.GetTexture("Bird Frame 3"));
		_animationFrames.push_back(this->_data->assets.GetTexture("Bird Frame 4"));

		_birdSprite.setTexture(_animationFrames.at(_animationIterator));

		_birdSprite.setPosition((_data->window.getSize().x / 4) - (_birdSprite.getGlobalBounds().width / 2), (_data->window.getSize().y / 2) - (_birdSprite.getGlobalBounds().height / 2));
	
		_birdState = BIRD_STATE_STILL;

		sf::Vector2f origin = sf::Vector2f(_birdSprite.getGlobalBounds().width / 2, _birdSprite.getGlobalBounds().height / 2);

		_birdSprite.setOrigin(origin);

		_rotation = 0;
	}

	Bird::~Bird()
	{
		for (std::vector<Node*> layer : nodeNetwork)
		{
			for (Node* node : layer)
			{
				delete node;
			}
			layer.clear();
		}
		nodeNetwork.clear();
	}

	void Bird::Draw()
	{
		_data->window.draw(_birdSprite);
	}

	void Bird::Animate(float dt)
	{
		if (_clock.getElapsedTime().asSeconds() > BIRD_ANIMATION_DURATION / _animationFrames.size())
		{
			if (_animationIterator < _animationFrames.size() - 1)
			{
				_animationIterator++;
			}
			else
			{
				_animationIterator = 0;
			}

			_birdSprite.setTexture(_animationFrames.at(_animationIterator));

			_clock.restart();
		}
	}

	void Bird::Update(float dt)
	{
		if (BIRD_STATE_FALLING == _birdState)
		{
			_birdSprite.move(0, GRAVITY * GAME_SPEED * dt);

			_rotation += ROTATION_SPEED * dt;

			if (_rotation > 25.0f)
			{
				_rotation = 25.0f;
			}

			_birdSprite.setRotation(_rotation);
		}
		else if (BIRD_STATE_FLYING == _birdState)
		{
			_birdSprite.move(0, -FLYING_SPEED * GAME_SPEED * dt);
			if (_birdSprite.getPosition().y < 0) {
				sf::Vector2f v = _birdSprite.getPosition();
				v.y = 0;
				_birdSprite.setPosition(v);
			}

			_rotation -= ROTATION_SPEED * dt;

			if (_rotation < -25.0f)
			{
				_rotation = -25.0f;
			}

			_birdSprite.setRotation(_rotation);
		}

		if (_movementClock.getElapsedTime().asSeconds() > (FLYING_DURATION/GAME_SPEED))
		{
			_movementClock.restart();
			_birdState = BIRD_STATE_FALLING;
		}
	}

	void Bird::Tap()
	{
		_movementClock.restart();
		_birdState = BIRD_STATE_FLYING;
	}

	const sf::Sprite &Bird::GetSprite() const
	{
		return _birdSprite;
	}

	void Bird::getHeight(int& x, int& y)
	{
		sf::Vector2f v = _birdSprite.getPosition();
		x = (int)v.x;
		y = (int)v.y;
	}

	bool Bird::FindShouldFlap(float distanceToGround, float distanceToTop)
	{
		OutputNode output = OutputNode();
		for (int i = 0; i < nodeNetwork.size(); i++)
		{
			//Input layer receives inputs from the game
			if (i == 0)
			{
				nodeNetwork.at(i).at(2)->AddInput(distanceToGround);
				//nodeNetwork.at(i).at(3)->AddInput(distanceToTop);
			}

			//iterate nodes on current layer
			for (int j = 0; j < nodeNetwork.at(i).size(); j++)
			{
				//if last layer, send the output to the output node
				if (nodeNetwork.at(i).at(j)->lastLayer)
				{
					output.AddInput(nodeNetwork.at(i).at(j)->GenerateOutput());
				}
				//otherwise, iterate nodes in next layer, generating outputs for a particular node
				else
				{
					for (int k = 0; k < nodeNetwork.at(i + 1).size(); k++)
					{
						nodeNetwork.at(i + 1).at(k)->AddInput(nodeNetwork.at(i).at(j)->GenerateOutput(k));
					}
				}
			}

			//Clear values for next update
			for (int j = 0; j < nodeNetwork.at(i).size(); j++)
			{
				nodeNetwork.at(i).at(j)->ClearValue();
			}
		}
		float val = output.GenerateOutput();
		if (val < 0)
		{
			return false;
		}
		else {
			return true;
		}
	}

	bool Bird::FindShouldFlap(float fdistanceToPipe, float fdistanceToCentreOfPipe, float fdistanceToGround, float fdistanceToTop)
	{
		//Normalize the values
		float distanceToPipe = fdistanceToPipe / (SCREEN_WIDTH - 69);
		float distanceToGround = fdistanceToGround / 763;
		float distanceToCentreOfPipe = fdistanceToCentreOfPipe / 763;
		float state = 0.0f;
		if (_birdState == BIRD_STATE_FALLING)
			state = 1.0f;
		OutputNode output = OutputNode();
		for (int i = 0; i < nodeNetwork.size(); i++)
		{
			//Input layer receives inputs from the game
			if (i == 0)
			{
				nodeNetwork.at(i).at(0)->AddInput(distanceToPipe);
				nodeNetwork.at(i).at(1)->AddInput(distanceToCentreOfPipe);
				nodeNetwork.at(i).at(2)->AddInput(distanceToGround);
				nodeNetwork.at(i).at(3)->AddInput(state);
			}

			//iterate nodes on current layer
			for (int j = 0; j < nodeNetwork.at(i).size(); j++)
			{
				//if last layer, send the output to the output node
				if (nodeNetwork.at(i).at(j)->lastLayer)
				{
					output.AddInput(nodeNetwork.at(i).at(j)->GenerateOutput());
				}
				//otherwise, iterate nodes in next layer, generating outputs for a particular node
				else
				{
					for (int k = 0; k < nodeNetwork.at(i + 1).size(); k++)
					{
						nodeNetwork.at(i + 1).at(k)->AddInput(nodeNetwork.at(i).at(j)->GenerateOutput(k));
					}
				}
			}

			//Clear values for next update
			for (int j = 0; j < nodeNetwork.at(i).size(); j++)
			{
				nodeNetwork.at(i).at(j)->ClearValue();
			}
		}
		float val = output.GenerateOutput();
		if (val < 0)
		{
			return false;
		}
		else {
			return true;
		}
	}
}