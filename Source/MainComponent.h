#pragma once

#include <JuceHeader.h>

#include "HtmlBrowser.h"

class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::unique_ptr< SimpleHtmlBrowser> browser;
    std::unique_ptr<SquareButton> btnOpen;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
