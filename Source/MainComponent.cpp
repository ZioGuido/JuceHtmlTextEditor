#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    btnOpen.reset(new SquareButton("Open browser", 1));
    addAndMakeVisible(btnOpen.get());
    btnOpen->onClickCallback = [&](const MouseEvent&) { browser->Show(); };
    btnOpen->centreWithSize(120, 30);

    browser.reset(new SimpleHtmlBrowser());
    addChildComponent(browser.get());

    setSize(1024, 768);

    browser->Show();
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (Colours::black);
}

void MainComponent::resized()
{
    browser->setBounds(0, 0, getWidth(), getHeight());
}
