#include "JUCE/JuceHeader.h"
#include "GUI.h"
#include "reaperHelpers.h"
#include "EDL_Conformer.h"


class ConformerComponent : public BaseWwiseGuiComponent
{
public:
	ConformerComponent()
	{
		conformer = new EDLconformer;
	};
	
	~ConformerComponent()
	{
		delete conformer;
	};

private:
	EDLconformer* conformer = nullptr;
	
	void resized() override;
	
	void buttonClicked(juce::Button* pButton)override;
	
	std::vector<juce::Button*> buttons{
	};
	
	
};
