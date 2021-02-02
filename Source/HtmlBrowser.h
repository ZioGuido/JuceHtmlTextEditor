/*
  ==============================================================================

    SimpleHtmlBrowser.h
    Created: 31 Jan 2021
    Author:  Guido Scognamiglio - www.GenuineSoundware.com

    Not an actual browser! This is nothing but a component that hosts the 
    GSiHtmlTextEditor class to display some hypertext and handles links
    for internal functions.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GSiHtmlTextEdit.h"
#include "Common_UI.h"

//==============================================================================
class SimpleHtmlBrowser  : public juce::Component
{
public:
    SimpleHtmlBrowser()
    {
        setOpaque(true);

        htmlView.reset(new GSiHtmlTextEdit());
        htmlView->getPointerToTextEditorComponent()->setScrollbarsShown(true);
        htmlView->setNormalFontSize(15.f);
        htmlView->setBackgroundColor(Colour(0xFF404050));
        addAndMakeVisible(htmlView.get());
        
        btnClose.reset(new SquareButton("CLOSE", 1));
        btnClose->onClickCallback = [&](const MouseEvent&){ Hide(); };
        addAndMakeVisible(btnClose.get());

        btnBack.reset(new SquareButton("<< BACK", 1));
        btnBack->onClickCallback = [&](const MouseEvent&) { Back(); };
        btnBack->setEnabled(false);
        addAndMakeVisible(btnBack.get());

        dialog.reset(new GSiDialogWindow());
        addChildComponent(dialog.get());

        // Define the lambda that will be called when an internal link is clicked
        htmlView->internalLinkFunction = [&](const String& s)
        {
            if (s.startsWithIgnoreCase("#alert="))
            {
                String msg = s.fromFirstOccurrenceOf("#alert=", false, true);
                dialog->Open(GSiDialogWindow::AlertType::typeInfo, "You got a message!", msg);
            }
            else
                LoadPage(s);
        };
    }

    ~SimpleHtmlBrowser() override
    {
    }

    void Show()
    {
        history.clear();
        LoadPage("page1.htm");
        Desktop::getInstance().getAnimator().fadeIn(this, 250);
    }

    void Hide()
    {
        Desktop::getInstance().getAnimator().fadeOut(this, 250);
    }

    void Back()
    {
        if (history.size() == 1) return;
        if (history.size() > 1) history.removeLast();
        LoadPage(history.getLast(), true);
    }

    void LoadPage(const String& page, bool goingBack = false)
    {
        if (!goingBack) history.add(page);
        btnBack->setEnabled(history.size() > 1);

        String resource = page.replace(".", "_");
        int FileSize = 0;
        
        // Get the file from the resources
        String HTML = String(BinaryData::getNamedResource(resource.toRawUTF8(), FileSize));
        
        if (FileSize <= 0) return;

        htmlView->Reset();
        htmlView->appendHtml(HTML);
        htmlView->getPointerToTextEditorComponent()->moveCaretToTop(false);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll(Colours::black);
    }

    void resized() override
    {
        htmlView->setBounds(0, 0, getWidth(), getHeight() - 40);
        btnClose->setBounds(getWidth() - 140, getHeight() - 35, 120, 30);
        btnBack->setBounds(getWidth() - 270, getHeight() - 35, 120, 30);
    }

private:
    std::unique_ptr<GSiHtmlTextEdit> htmlView;
    std::unique_ptr<SquareButton> btnClose, btnBack;
    std::unique_ptr<GSiDialogWindow> dialog;

    Array<String> history;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleHtmlBrowser)
};
