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
	while (std::abs(weight) < 0.001f) {
		weight = -0.7f + ((float)rand() / ((float)RAND_MAX / (0.7f - -0.7f)));
	}
}

float InputNode::GenerateOutput()
{
	return sum * weight;
}

ActivationNode::ActivationNode()
{
	//Generate random weight and bias
	while (std::abs(weight) < 0.001f) {
		weight = -0.7f + ((float)rand() / ((float)RAND_MAX / (0.7f - -0.7f)));
	}
	bias = -0.7f + ((float)rand() / ((float)RAND_MAX / (0.7f - -0.7f)));
}

float ActivationNode::GenerateOutput()
{
	//Apply TanH or ReLU function to the sum

	return (sum + bias) * weight;
}

float OutputNode::GenerateOutput()
{
	//Apply binary step function to the sum

	return sum;
}
