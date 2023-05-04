#include "Node.h"
#include "DEFINITIONS.hpp"
void Node::AddInput(float input)
{
	sum += input;
}

float Node::GenerateOutput()
{
	return sum;
}

InputNode::InputNode(bool p_lastLayer)
{
	lastLayer = p_lastLayer;
	//Generate random weights (not 0)
	if (!lastLayer) {
		for (int i = 0; i < NODES_PER_LAYER; i++)
		{
			float weight = 0;
			while (std::abs(weight) < 0.0001f) {
				weight = -WEIGHT_MAX + ((float)rand() / ((float)RAND_MAX / (WEIGHT_MAX - -WEIGHT_MAX)));
				weights.push_back(weight);
			}
		}
	}
	else {
		float weight = 0;
		while (std::abs(weight) < 0.0001f) {
			weight = -WEIGHT_MAX + ((float)rand() / ((float)RAND_MAX / (WEIGHT_MAX - -WEIGHT_MAX)));
			weights.push_back(weight);
		}
	}
}

float InputNode::GenerateOutput()
{
	return sum * weights.at(0);
}

float InputNode::GenerateOutput(int nodeIndex)
{
	return sum * weights.at(nodeIndex);
}

ActivationNode::ActivationNode(bool p_lastLayer)
{
	lastLayer = p_lastLayer;
	if (!lastLayer) {
		//Generate random weights and bias
		for (int i = 0; i < NODES_PER_LAYER; i++)
		{
			float weight = 0;
			while (std::abs(weight) < 0.0001f) {
				weight = -WEIGHT_MAX + ((float)rand() / ((float)RAND_MAX / (WEIGHT_MAX - -WEIGHT_MAX)));
				weights.push_back(weight);
			}
		}
	}
	//Generate just one weight
	else {
		float weight = 0;
		while (std::abs(weight) < 0.0001f) {
			weight = -WEIGHT_MAX + ((float)rand() / ((float)RAND_MAX / (WEIGHT_MAX - -WEIGHT_MAX)));
			weights.push_back(weight);
		}
	}
	bias = -WEIGHT_MAX + ((float)rand() / ((float)RAND_MAX / (WEIGHT_MAX - -WEIGHT_MAX)));
}

float ActivationNode::GenerateOutput()
{
	//add bias
	float val = sum + bias;
	//Apply TanH or ReLU function 
	val = std::tanh(val);

	return val * weights.at(0);
}

float ActivationNode::GenerateOutput(int nodeIndex)
{
	//add bias
	float val = sum + bias;
	//Apply TanH or ReLU function 
	val = std::tanh(val);

	return val * weights.at(nodeIndex);
}

float OutputNode::GenerateOutput()
{
	float val = sum;
	//Apply binary step function. (-1 = false, 1 = true)
	if (val <= 0)
		val = -1;
	else if (val > 0)
		val = 1;
	return sum;
}
