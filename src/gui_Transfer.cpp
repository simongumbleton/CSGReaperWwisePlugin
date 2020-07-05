#include "gui_Transfer.h"




StringArray TransferToWwiseComponent::ToJuceStringArray(std::vector<std::string> strings)
{
	StringArray output;
	for (auto string : strings)
	{
		output.add(string);
	}
	return output;
}


TransferToWwiseComponent::TransferToWwiseComponent() //constructor
{

	waapi_Connect(MyCurrentWwiseConnection);
	
	InitAllButtons(buttons);

	//InitComboBox(dd_Language, parentWwiseConnectionHnd->MyCurrentWwiseConnection.projectGlobals.Languages)
	InitComboBox(dd_EventOption, myCreateChoices.waapiCREATEchoices_EVENTOPTIONS);
	InitComboBox(dd_CreateType,myCreateChoices.waapiCREATEchoices_TYPE);
	InitComboBox(dd_OnNameConflict,myCreateChoices.waapiCREATEchoices_NAMECONFLICT);



	tree_RenderJobTree->setRootItem(nullptr);
	tree_RenderJobTree->setDefaultOpenness(true);
	tree_RenderJobTree->setMultiSelectEnabled(true);

	juce::Identifier renderJobType("ARenderJob");
	juce::ValueTree renderJobNode(renderJobType);

	tree_RenderJobTree->setRootItem(renderJobNode);


	addAndMakeVisible(tree_RenderJobTree);

	addAndMakeVisible(txt_ConnectionStatus);

	debugLabel->setText("debug", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(debugLabel);


	setSize(500, 500);

}

void TransferToWwiseComponent::InitAllButtons(std::vector<juce::Button *> buttons)
{
	for (auto button : buttons)
	{
		String name = button->getName();
		button->setButtonText(button->getName());
		button->addListener(this);
		addAndMakeVisible(button);
	}
}

void TransferToWwiseComponent::InitComboBox(juce::ComboBox * comboBox, std::vector<std::string> choices)
{
	comboBox->addItemList(ToJuceStringArray(choices),1);
	comboBox->addListener(this);
	addAndMakeVisible(comboBox);
}


void TransferToWwiseComponent::resized()
{
	auto border = 4;
	auto area = getLocalBounds();

	auto LeftHalf = area.removeFromLeft(area.getWidth() / 2);
	auto RightHalf = area;

	auto buttonHeight = 30;
	for (auto button : buttons)
	{
		button->setBounds(LeftHalf.removeFromTop(buttonHeight).reduced(border));
	}

	auto comboHeight = 30;
	for (auto cb : comboBoxes)
	{
		cb->setBounds(RightHalf.removeFromTop(comboHeight).reduced(border));
	}

	auto treeHeight = 50;

	tree_RenderJobTree->setBounds(LeftHalf.removeFromTop(treeHeight).reduced(border));

	auto labelHeight = 20;
	txt_ConnectionStatus->setBounds(RightHalf.removeFromBottom(labelHeight));
	debugLabel->setBounds(RightHalf.removeFromBottom(labelHeight));
}

void TransferToWwiseComponent::buttonClicked(juce::Button * pButton)
{
	pButton->setColour(juce::Label::textColourId, juce::Colours::aqua);
	String text = ("CLICKED: " + pButton->getButtonText());
	//debug
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);

	if (pButton == btn_ConnectToWwise)
	{
		bool connected = waapi_Connect(MyCurrentWwiseConnection);
		if (connected)
		{
			String text = ("Wwise Connected: " + MyCurrentWwiseConnection.Version);
			txt_ConnectionStatus->setText(text, juce::NotificationType::dontSendNotification);
		}
		else
		{
			txt_ConnectionStatus->setText("No wwise connection", juce::NotificationType::dontSendNotification);
		}
	}

	
}

void TransferToWwiseComponent::comboBoxChanged(ComboBox * comboBoxThatHasChanged)
{
	String text = ("SELECTED: " + comboBoxThatHasChanged->getText());
	debugLabel->setText(text, juce::NotificationType::dontSendNotification);
}

void TransferToWwiseComponent::labelTextChanged(Label * labelThatHasChanged)
{
}
