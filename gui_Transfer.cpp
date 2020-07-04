#include "gui_Transfer.h"

TransferToWwiseComponent::TransferToWwiseComponent()
{
	
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
	}


	debugLabel.setText("debug", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(debugLabel);


	setSize(500, 500);

}


void TransferToWwiseComponent::resized()
{
	auto border = 4;
	auto area = getLocalBounds();

	auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);

	auto buttonHeight = 30;
	for (auto button : buttons)
	{
		button->setBounds(LeftHalf.removeFromTop(buttonHeight).reduced(border));
	}
	auto labelHeight = 20;
	debugLabel.setBounds(area.removeFromBottom(labelHeight));
}

void TransferToWwiseComponent::buttonClicked(juce::Button * pButton)
{
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug
	debugLabel.setText(text, juce::NotificationType::dontSendNotification);
	
}

void TransferToWwiseComponent::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
}

void TransferToWwiseComponent::labelTextChanged(Label * labelThatHasChanged)
{
}
