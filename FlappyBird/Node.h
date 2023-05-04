#pragma once
#include <cmath>
#include <vector>
class Node
{
public:
	Node() { lastLayer = false; }
	Node(bool p_lastLayer) : lastLayer(p_lastLayer) { }

	virtual void AddInput(float input);

	virtual float GenerateOutput();
	virtual float GenerateOutput(int nodeIndex) { return 0; }

	void ClearValue() { sum = 0; }

	bool lastLayer;

protected:
	float sum = 0;
};

class InputNode : public Node
{
public:
	InputNode(bool p_lastLayer);

	InputNode(std::vector<float> p_weights, bool p_lastLayer) { weights = std::vector<float>(p_weights); lastLayer = p_lastLayer; }
	
	float GenerateOutput() override;
	float GenerateOutput(int nodeIndex) override;

	std::vector<float> weights;

};

class ActivationNode : public Node
{
public:
	ActivationNode(bool p_lastLayer);

	ActivationNode(std::vector<float> p_weights, float p_bias, bool p_lastLayer) : bias(p_bias) { weights = std::vector<float>(p_weights); lastLayer = p_lastLayer; }

	float GenerateOutput() override;
	float GenerateOutput(int nodeIndex) override;

	std::vector<float> weights;
	float bias = 0;
};

class OutputNode : public Node
{
public:
	float GenerateOutput() override;
};

