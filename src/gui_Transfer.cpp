#include "gui_Transfer.h"

TransferToWwiseComponent::TransferToWwiseComponent()
{
	
	for (auto button : buttons)
	{
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
	}


	setSize(500, 500);

}

TransferToWwiseComponent::~TransferToWwiseComponent()
{
	delete this;
}


void TransferToWwiseComponent::resized()
{
	auto border = 4;
	auto area = getLocalBounds();
	auto buttonHeight = 30;
	for (auto button : buttons)
	{
		button->setBounds(area.removeFromTop(buttonHeight).reduced(border));
	}
}
