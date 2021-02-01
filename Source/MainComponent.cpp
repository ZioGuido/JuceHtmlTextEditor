#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (800, 600);

    btnOpen.reset(new SquareButton("Open browser", 1));
    addAndMakeVisible(btnOpen.get());
    btnOpen->onClickCallback = [&](const MouseEvent&) { browser->Show(); };
    btnOpen->centreWithSize(120, 30);

    browser.reset(new SimpleHtmlBrowser());
    browser->setBounds(0, 0, getWidth(), getHeight());
    addChildComponent(browser.get());

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
}
