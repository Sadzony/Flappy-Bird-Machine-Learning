#include "Node.h"

void Node::AddInput(float input)
{
	sum += input;
}

float Node::GenerateOutput()
{
	return sum;
}

InputNode::InputNode()
{
	//Generate random weight (not 0)
	while (std::abs(weight) < 0.0001f) {
		weight = -1.5f + ((float)rand() / ((float)RAND_MAX / (1.5f - -1.5f)));
	}
}

float InputNode::GenerateOutput()
{
	return sum * weight;
}

ActivationNode::ActivationNode()
{
	//Generate random weight and bias
	while (std::abs(weight) < 0.0001f) {
		weight = -1.5f + ((float)rand() / ((float)RAND_MAX / (1.5f - -1.5f)));
	}
	bias = -1.5f + ((float)rand() / ((float)RAND_MAX / (1.5f - -1.5f)));
}

float ActivationNode::GenerateOutput()
{
	//add bias
	float val = sum + bias;
	//Apply TanH or ReLU function 
	val = std::tanh(val);

	return val * weight;
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
