/*
  ==============================================================================

    SimpleHtmlBrowser.h
    Created: 31 Jan 2021
    Author:  Guido Scognamiglio - www.GenuineSoundware.com

    Last update: 09 June 2021

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
class SimpleHtmlBrowser  : public juce::Component, juce::KeyListener
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
        //htmlView->setShowAnchorPopup(false);
        //htmlView->setMobileStyle(true);
        //htmlView->useImageIdents = true;
        


        btnLoad.reset(new SquareButton("Load HTML", 1));
        btnLoad->onClickCallback = [&](const MouseEvent&) 
        {
            fileChooser.reset(new FileChooser("Load HTML file", File::getCurrentWorkingDirectory(), "*.htm;*.html", true));
            fileChooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                [&](const FileChooser& fc) mutable
                {
                    if (fc.getURLResults().size() > 0)
                    {
                        history.clear();
                        auto HTML = fc.getResult().loadFileAsString();

                        htmlView->Reset(true);
                        htmlView->appendHtml(HTML);
                        htmlView->getPointerToTextEditorComponent()->moveCaretToTop(false);
                    }
                });
        };
        addAndMakeVisible(btnLoad.get());

        btnBack.reset(new SquareButton("<< BACK", 1));
        btnBack->onClickCallback = [&](const MouseEvent&) { Back(); };
        btnBack->setEnabled(false);
        addAndMakeVisible(btnBack.get());
        
        btnSearch.reset(new SquareButton("Search", 1));
        btnSearch->onClickCallback = [&](const MouseEvent&) { DoSearch(); };
        addAndMakeVisible(btnSearch.get());

        searchField.reset(new Label("searchField", "Search..."));
        searchField->setEditable(true, false, false);
        //searchField->onTextChange = [&] { DoSearch(); };
        addAndMakeVisible(searchField.get());

        btnExport.reset(new SquareButton("EXPORT", 1));
        btnExport->onClickCallback = [&](const MouseEvent&)
        {
            fileChooser.reset(new FileChooser(translate("Save to..."), File::getSpecialLocation(File::userDesktopDirectory).getFullPathName(), "*.png", true));
            fileChooser->launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::warnAboutOverwriting,
                [this](const FileChooser& fc) mutable
                {
                    if (fc.getURLResults().size() > 0)
                        htmlView->exportPageToImage(fc.getURLResult());
                }
            );
        };
        addAndMakeVisible(btnExport.get());

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

        addKeyListener(this);
        setWantsKeyboardFocus(true);
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

#if JUCE_WINDOWS && _DEBUG
        // Load HTML from file in DEBUG mode
        String HTML = File::getCurrentWorkingDirectory().getChildFile("../../Source/Resources/" + page).loadFileAsString();
#else
        // Get the file from the resources
        String resource = page.replace(".", "_");
        int FileSize = 0;
        String HTML = String(BinaryData::getNamedResource(resource.toRawUTF8(), FileSize));
#endif
        if (HTML.isEmpty()) return;

        htmlView->Reset(true);
        htmlView->appendHtml(HTML);
        htmlView->getPointerToTextEditorComponent()->moveCaretToTop(false);
    }

    void DoSearch()
    {
        auto text = searchField->getText();
        DBG("Search for: " << text);
        htmlView->searchAndHighlight(text);
    }

private:
    std::unique_ptr<GSiHtmlTextEdit> htmlView;
    std::unique_ptr<SquareButton> btnLoad, btnBack, btnSearch, btnExport;
    std::unique_ptr<GSiDialogWindow> dialog;
    std::unique_ptr<Label> searchField;

    Array<String> history;

    std::unique_ptr<FileChooser> fileChooser;

    void paint(juce::Graphics& g) override
    {
        g.fillAll(Colours::black);
    }

    void resized() override
    {
        btnLoad->setBounds(0, 0, 100, 30);
        btnBack->setBounds(110, 0, 100, 30);
        searchField->setBounds(220, 0, getWidth() - 220 - 230, 30);
        btnSearch->setBounds(getWidth() - 220, 0, 100, 30);
        btnExport->setBounds(getWidth() - 110, 0, 100, 30);

        htmlView->setBounds(0, 35, getWidth(), getHeight() - 40);
    }

    bool keyPressed(const KeyPress& key, Component* originatingComponent) override
    {
#if (_DEBUG && JUCE_WINDOWS)
        if (key == key.F5Key)
        {
            LoadPage("page1.htm");
        }
#endif
        return true;
    }


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleHtmlBrowser)
};
