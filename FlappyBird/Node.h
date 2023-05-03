#pragma once
#include <cmath>
class Node
{
public:
	Node() { }

	virtual void AddInput(float input);

	virtual float GenerateOutput();

	void ClearValue() { sum = 0; }

protected:
	float sum = 0;
};

class InputNode : public Node
{
public:
	InputNode();

	InputNode(float p_weight) : weight(p_weight) { }
	
	float GenerateOutput() override;

	float weight = 0;
};

class ActivationNode : public Node
{
public:
	ActivationNode();

	ActivationNode(float p_weight, float p_bias) : weight(p_weight), bias(p_bias) { }

	float GenerateOutput() override;

	float weight = 0;
	float bias = 0;
};

class OutputNode : public Node
{
public:
	OutputNode() { }
	float GenerateOutput() override;
};

