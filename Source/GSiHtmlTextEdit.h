/*
  ==============================================================================

    GSiHtmlTextEdit.h
    Created: 29 Jan 2021 6:32:02pm
    Author:  Guido Scognamiglio - www.GenuineSoundware.com

    Uses a TextEditor component and attempts to parse some simple HTML4 to 
    easily format text with different sizes, colors, styles, fonts and also 
    inserts hyperlinks with cursor change and a floating tooltip when the 
    mouse pointer is over the links.
    Of course this supports only some basic HTML4 tags, no CSS and no JS. 

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================

class GSiHtmlTextEdit  : public juce::Component
{
public:
    GSiHtmlTextEdit()
    {
        textEditor.reset(new TextEditor());
        textEditor->setScrollbarsShown(false);
        textEditor->setMultiLine(true, true);
        textEditor->setReadOnly(true);
        textEditor->setPopupMenuEnabled(false);
        textEditor->setLineSpacing(1.1f);
        textEditor->setColour(TextEditor::ColourIds::outlineColourId, Colour(0));
        textEditor->setColour(TextEditor::ColourIds::focusedOutlineColourId, Colour(0));
        addAndMakeVisible(textEditor.get());

        // Set defaults
        Reset(true);

        // Used to catch mouse movement
        addMouseListener(this, true);
    }

    ~GSiHtmlTextEdit() override
    {
    }

    void Reset(bool fullReset = false)
    {
        charCounter = 0; // Character count
        AllLinks.clear();
        
        textEditor->clear();
        textEditor->setCaretPosition(0);

        if (fullReset)
        {
            fontFace = prev_fontFace = "Verdana";
            fontSize = prev_fontSize = 18.f;
            fontColor = prev_fontColor = Colours::white;
            fontStyle = Font::FontStyleFlags::plain;
            textEditor->setFont(Font(fontFace, fontSize, fontStyle));
            textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
            textEditor->setColour(TextEditor::ColourIds::backgroundColourId, Colours::black);
        }
    }

    //==============================================================================

    void setNormalFontSize(float size)
    {
        fontSize = prev_fontSize = size;
        textEditor->setFont(Font(fontFace, fontSize, fontStyle));
    }

    void setNormalFontColor(const Colour& col)
    {
        fontColor = prev_fontColor = col;
        textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
    }

    void setNormalFontFace(const String& face)
    {
        fontFace = prev_fontFace = face;
        textEditor->setFont(Font(fontFace, fontSize, fontStyle));
    }

    void setBackgroundColor(const Colour& col)
    {
        textEditor->setColour(TextEditor::ColourIds::backgroundColourId, col);
    }

    void setLinkColor(const Colour& col)
    {
        linkColor = col;
    }

    
    //==============================================================================

    // Use this in case you need to access TextEditor's members from the outside
    TextEditor* getPointerToTextEditorComponent()
    {
        return textEditor.get();
    }

    // Parse and add some HTML to the TextEditor component
    void appendHtml(const String& HTML)
    {
        String input = HTML.replace("\r\n", "\n").replace("\n", "");

        bool beginTag = false, canParse = false;
        String tag, output;

        for (auto s : input)
        {
            if (s == '<')
            {
                // Add the text already rendered
                textEditor->setCaretPosition(charCounter); 
                textEditor->insertTextAtCaret(output);

                beginTag = true;
                canParse = false;
                continue;
            }

            if (beginTag)
            {
                if (s == '>')
                {
                    // Clear the output buffer
                    output.clear();

                    beginTag = false;
                    canParse = true;
                }
                else 
                {
                    tag += s;
                    continue;
                }
            }

            if (canParse && tag.isNotEmpty())
            {
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Break line
                if (tag == "br") { textEditor->insertTextAtCaret("\n"); charCounter++; }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Italic
                else if (tag ==  "i") { fontStyle |= Font::FontStyleFlags::italic;      textEditor->setFont(Font(fontFace, fontSize, fontStyle)); }
                else if (tag == "/i") { fontStyle ^= Font::FontStyleFlags::italic;      textEditor->setFont(Font(fontFace, fontSize, fontStyle)); }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Bold
                else if (tag ==  "b") { fontStyle |= Font::FontStyleFlags::bold;        textEditor->setFont(Font(fontFace, fontSize, fontStyle)); }
                else if (tag == "/b") { fontStyle ^= Font::FontStyleFlags::bold;        textEditor->setFont(Font(fontFace, fontSize, fontStyle)); }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Underlined
                else if (tag ==  "u") { fontStyle |= Font::FontStyleFlags::underlined;  textEditor->setFont(Font(fontFace, fontSize, fontStyle)); }
                else if (tag == "/u") { fontStyle ^= Font::FontStyleFlags::underlined;  textEditor->setFont(Font(fontFace, fontSize, fontStyle)); }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Anchor
                else if (tag.startsWithIgnoreCase("a "))
                {
                    prev_fontColor = fontColor;
                    fontStyle = Font::FontStyleFlags::underlined;
                    textEditor->setFont(Font(fontFace, fontSize, fontStyle));
                    textEditor->setColour(TextEditor::ColourIds::textColourId, linkColor);

                    if (tag.containsIgnoreCase("href"))
                    {
                        tmpHL.url = tag.fromFirstOccurrenceOf("href=", false, true).replace("\"", "");
                        tmpHL.position.setStart(charCounter);
                    }
                }
                else if (tag == "/a") 
                { 
                    fontStyle ^= Font::FontStyleFlags::underlined;  
                    textEditor->setFont(Font(fontFace, fontSize, fontStyle)); 
                    textEditor->setColour(TextEditor::ColourIds::textColourId, prev_fontColor);

                    tmpHL.position.setEnd(charCounter);
                    AllLinks.add(tmpHL);
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Font
                else if (tag.startsWithIgnoreCase("font "))
                {
                    if (tag.containsIgnoreCase("size"))
                    {
                        prev_fontSize = fontSize;
                        int i1 = tag.indexOf("size=") + 5; int i2 = tag.indexOfChar(i1 + 1, '"');
                        auto val = tag.substring(i1, i2).replace("\"", "");
                        fontSize = val.getFloatValue();
                        textEditor->setFont(Font(fontFace, fontSize, fontStyle));
                    }

                    if (tag.containsIgnoreCase("color"))
                    {
                        prev_fontColor = fontColor;
                        int i1 = tag.indexOf("color=") + 6; int i2 = tag.indexOfChar(i1 + 1, '"');
                        auto val = tag.substring(i1, i2).replace("#", "");
                        auto col = val.getHexValue32() + 0xFF000000;
                        fontColor = Colour(col);
                        textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
                    }

                    if (tag.containsIgnoreCase("face"))
                    {
                        prev_fontFace = fontFace;
                        int i1 = tag.indexOf("face=") + 6; int i2 = tag.indexOfChar(i1 + 1, '"');
                        fontFace = tag.substring(i1, i2).replace("\"", "");
                        textEditor->setFont(Font(fontFace, fontSize, fontStyle));
                    }
                }
                else if (tag == "/font")
                {
                    fontFace = prev_fontFace;
                    fontSize = prev_fontSize;
                    textEditor->setFont(Font(fontFace, fontSize, fontStyle));

                    fontColor = prev_fontColor;
                    textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // List Item
                else if (tag.startsWithIgnoreCase("li"))
                {
                    String indent("\n  - ");
                    textEditor->insertTextAtCaret(indent);
                    charCounter += indent.length();
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Headers
                else if (tag.startsWithIgnoreCase("h") && tag.containsAnyOf("1234"))
                {
                    prev_fontSize = fontSize;
                    auto sz = tag.getLastCharacters(1).getIntValue();
                    fontSize = 40 - sz * 4;
                    textEditor->setFont(Font(fontFace, fontSize, fontStyle));

                    // Add newline before header text
                    textEditor->insertTextAtCaret("\n"); charCounter += 1;
                }

                else if (tag.startsWithIgnoreCase("/h") && tag.containsAnyOf("1234"))
                {
                    fontSize = prev_fontSize;
                    textEditor->setFont(Font(fontFace, fontSize, fontStyle));

                    // Add newline after header text
                    textEditor->insertTextAtCaret("\n"); charCounter += 1;
                }
                
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                tag.clear();
                continue;
            }

            // Add the plain characters to the output buffer
            output += String::charToString(s);
            charCounter++;
        }

        // Print the remaining output buffer
        textEditor->setCaretPosition(charCounter);
        textEditor->insertTextAtCaret(output);
    }


    //==============================================================================

    void paintOverChildren(juce::Graphics& g) override
    {
        if (hoverLink)
        {
            g.setFont(toolTipFont);
            g.setColour(Colours::beige);
            g.fillRoundedRectangle(hoverPosition.x, hoverPosition.y, toolTipWidth, 25, 5.f);
            g.setColour(Colours::black);
            g.drawText(hoverLinkText, hoverPosition.x, hoverPosition.y, toolTipWidth, 25, Justification::centred);
        }
    }

    void resized() override
    {
        textEditor->setBounds(0, 0, getWidth(), getHeight());
    }

    void mouseMove(const MouseEvent& event)
    {
        if (textEditor->getMouseCursor() != MouseCursor::NormalCursor)
        {
            textEditor->setMouseCursor(MouseCursor::NormalCursor);
            hoverLink = false;
            repaint();
        }

        for (auto l : AllLinks)
        {
            int i = textEditor->getTextIndexAt(event.x, event.y);
            if (l.position.contains(i))
            {
                textEditor->setMouseCursor(MouseCursor::PointingHandCursor);
                if (l.url.startsWithChar('#')) break;

                hoverLink = true;
                hoverLinkText = l.url;
                toolTipWidth = toolTipFont.getStringWidth(hoverLinkText) + 10;
                hoverPosition = event.getPosition() + juce::Point<int>(15, 15);
                repaint();
                break;
            }
        }
    }

    void mouseUp(const MouseEvent& event)
    {
        for (auto l : AllLinks)
        {
            int i = textEditor->getTextIndexAt(event.x, event.y);
            if (l.position.contains(i))
            {
                if (l.url.startsWithIgnoreCase("http"))
                {
                    URL w(l.url);
                    w.launchInDefaultBrowser();
                }
                else if (l.url.startsWithChar('#'))
                {
                    if (internalLinkFunction != nullptr)
                        internalLinkFunction(l.url);
                }
                break;
            }
        }
    }


    //==============================================================================

    // Set a lambda that will be called whenever an internal link is clicked
    std::function<void(const String&)> internalLinkFunction;


private:
    std::unique_ptr<TextEditor> textEditor;

    int charCounter;
    String fontFace, prev_fontFace;
    float fontSize, prev_fontSize;
    int fontStyle = Font::FontStyleFlags::plain;
    Colour fontColor, prev_fontColor, linkColor = Colours::yellow;
    
    bool hoverLink = false;
    String hoverLinkText = String();
    juce::Point<int> hoverPosition;
    Font toolTipFont = Font("Arial", 14.f, Font::FontStyleFlags::plain);
    int toolTipWidth = 100;

    struct HyperLink
    {
        String url;
        Range<int> position;
    } tmpHL;
    Array<HyperLink> AllLinks;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GSiHtmlTextEdit)
};
