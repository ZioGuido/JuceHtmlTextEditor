/*
  ==============================================================================

    GSiHtmlTextEdit.h
    Author:  Guido Scognamiglio - www.GenuineSoundware.com
    Created: 29 Jan 2021 6:32:02pm
    Last Update: 20 Nov 2025

    Uses a TextEditor component and attempts to parse some simple HTML4 to
    easily format text with different sizes, colors, styles, fonts and also
    inserts hyperlinks with cursor change and a floating tooltip when the
    mouse pointer is over the links.
    Of course this supports only some basic HTML4 tags, no CSS and no JS.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#if JUCE_WINDOWS && JUCE_MAJOR_VERSION >= 8 && JUCE8_USE_SOFTWARE_RENDERER // JUCE 8.0.0 or later
 #define IMAGE_FROM_DATA_SIZE SoftwareImageType().convert(ImageCache::getFromMemory(data, size))
#else
 #define IMAGE_FROM_DATA_SIZE ImageCache::getFromMemory(data, size)
#endif


//==============================================================================

class GSiHtmlTextEdit : public juce::Component
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
        textEditor->setIndents(5, 20);
        textEditor->setColour(TextEditor::ColourIds::outlineColourId, Colour(0));
        textEditor->setColour(TextEditor::ColourIds::focusedOutlineColourId, Colour(0));
        //textEditor->setColour(TextEditor::ColourIds::highlightColourId, Colours::transparentBlack);
        textEditor->setColour(TextEditor::ColourIds::highlightColourId, findColour(TextEditor::ColourIds::backgroundColourId).contrasting());
        textEditor->setColour(TextEditor::ColourIds::highlightedTextColourId, findColour(TextEditor::ColourIds::highlightColourId).contrasting());
        addAndMakeVisible(textEditor.get());

        // Set defaults
        Reset(true);
        setShowAnchorPopup(true);
        setMobileStyle(false);

        // Used to catch mouse movement
        addMouseListener(this, true);
    }

    void Reset(bool fullReset = false)
    {
        charCounter = 0; // Character count
        lastSearchEndIndex = 0;
        lastSearchString.clear();
        AllLinks.clear();
        ImageComponents.clear();

        textEditor->clear();
        textEditor->setCaretPosition(0);
        if (mobileStyle) 
            textEditor->setBounds(textEditor->getBounds().withHeight(0));

        Comment = false;

        if (fullReset)
        {
            fontFace = prev_fontFace = "Verdana";
            fontSize = prev_fontSize = 18.f;
            fontColor = prev_fontColor = Colours::white;
            fontStyle = Font::FontStyleFlags::plain;
            doSetFont();
            textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
            //textEditor->setColour(TextEditor::ColourIds::backgroundColourId, Colours::black);
        }
    }

    //==============================================================================

    void setNormalFontSize(float size)
    {
        fontSize = prev_fontSize = size;
        doSetFont();
    }

    void setNormalFontColor(const Colour& col)
    {
        fontColor = prev_fontColor = col;
        textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
    }

    void setNormalFontFace(const String& face)
    {
        fontFace = prev_fontFace = face;
        doSetFont();
    }

    void setBackgroundColor(const Colour& col)
    {
        textEditor->setColour(TextEditor::ColourIds::backgroundColourId, col);
    }

    void setLinkColor(const Colour& col)
    {
        linkColor = col;
    }

    void setShowAnchorPopup(bool show)
    {
        showAnchorPopup = show;
    }

    // Call this to 1) disable text hightlight and 2) have drag scroll. You should also disable the scroll bars.
    void setMobileStyle(bool set)
    {
        mobileStyle = set;

        /*
        * In order to obtain a mobile style read-only TextEditor, we should:
        * 1) disable the scroll by scroll bar or mouse wheel
        * 2) disable text hightlight
        * Which are two things still not available for the TextEditor component as of Juce 6.0.8.
        * The trick is to put the entire TextEditor into an external ViewPort and have the
        * TextEditor component to cover the entire text height; and in order to disable normal
        * mouse interaction and just have drag scroll, a transparent component will be overlaid.
        */

        if (mobileStyle)
        {
            transparentLayer.reset(new Component());

            textEditor->addAndMakeVisible(transparentLayer.get());
            mobileStyleViewPort.setViewedComponent(textEditor.get(), false);
            mobileStyleViewPort.setScrollBarsShown(false, false, true, false);
            mobileStyleViewPort.setScrollOnDragMode(Viewport::ScrollOnDragMode::all);
            addAndMakeVisible(mobileStyleViewPort);
        }
        resized();
    }

    void setVerticalScrollBar(bool shouldBeVisible)
    {
        if (mobileStyle)
        {
            textEditor->setScrollbarsShown(false);
            mobileStyleViewPort.setScrollBarsShown(shouldBeVisible, false, true, false);
        }
        else
        {
            textEditor->setScrollbarsShown(shouldBeVisible);
            mobileStyleViewPort.setScrollBarsShown(false, false, true, false);
        }
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
        bool beginTag = false, canParseTag = false;
        bool beginEncoded = false, canParseEncoding = false;
        bool renderPreFormatted = false;
        String tag, code, output;
        juce_wchar lastChar = 0;
        ImagesInThisDocument.clear();

        // Replace CRLF with just LF
        String input = HTML.replace("\r\n", "\n");

        // Parse each single character in the HTML text
        for (auto s : input)
        {
            // Make sure that content in a pre-formatted paragraph passes unaltered, including HTML code, until the closing tag
            if (renderPreFormatted)
            {
                // Make a tab with 4 spaces
                if (s == '\t')
                {
                    output += "   ";
                    charCounter += 4;
                }
                else
                {
                    output += String::charToString(s);
                    charCounter++;
                }

                if (output.endsWithIgnoreCase("</pre>"))
                {
                    output = output.replace("</pre>", "", true);
                    charCounter -= 6;

                    textEditor->setCaretPosition(charCounter);
                    textEditor->insertTextAtCaret(output);
                    output.clear();

                    renderPreFormatted = false;
                    tag = "/pre";
                    canParseTag = true;
                }
                continue;
            }

            // Skip white spaces at the beginning of the text
            else
            {
                if (s == '\t')
                    continue;
            }

            // Catch HTML tag opening
            if (s == '<')
            {
                textEditor->setCaretPosition(charCounter);
                textEditor->insertTextAtCaret(output);
                output.clear();

                beginTag = true;
                canParseTag = false;
                continue;
            }

            // Read HTML tag or catch its closing
            if (beginTag)
            {
                if (s == '>')
                {
                    if (Comment) { if (!tag.endsWith("--")) continue; else Comment = false; } // Discard everything within a comment including tags
                    beginTag = false;
                    canParseTag = true;
                }
                else
                {
                    tag += s;
                    if (tag.startsWith("!--")) Comment = true; // Detect comment start
                    continue;
                }
            }

            // Catch HTML-Encoded character
            if (s == '&')
            {
                // Add the text already rendered
                textEditor->setCaretPosition(charCounter);
                textEditor->insertTextAtCaret(output);
                output.clear();

                beginEncoded = true;
                canParseEncoding = false;
                continue;
            }

            // Read HTML-Encoding or catch encoding closure
            if (beginEncoded)
            {
                if (s == ';')
                {
                    beginEncoded = false;
                    canParseEncoding = true;
                }
                else
                {
                    code += s;
                    continue;
                }
            }

            // Parse some common HTML-Encoded characters
            if (canParseEncoding && code.isNotEmpty())
            {
                if (code == "nbsp")     textEditor->insertTextAtCaret(" ");     // Non-breakable space
                if (code == "amp")      textEditor->insertTextAtCaret("&");     // Ampersand
                if (code == "quot")     textEditor->insertTextAtCaret("\"");    // Quotation mark
                if (code == "lt")       textEditor->insertTextAtCaret("<");
                if (code == "rt")       textEditor->insertTextAtCaret(">");
                if (code == "laquo")    textEditor->insertTextAtCaret(String::charToString(171));
                if (code == "raquo")    textEditor->insertTextAtCaret(String::charToString(187));
                if (code.startsWith("#")) textEditor->insertTextAtCaret(String::charToString(code.substring(1).getIntValue()));

                // A good solution would be to have a hash table with codes and their corresponding UNICODE characters...

                charCounter++;
                code.clear();
                continue;
            }

            // Parse Tags
            if (canParseTag && tag.isNotEmpty())
            {
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Break line
                if (tag.startsWithIgnoreCase("br ") || tag == "br") { lastChar = '\n'; textEditor->insertTextAtCaret("\n"); charCounter++; }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Italic
                else if (tag == "i" || tag == "em") { fontStyle |= Font::FontStyleFlags::italic;    doSetFont(); }
                else if (tag == "/i" || tag == "/em") { fontStyle ^= Font::FontStyleFlags::italic;    doSetFont(); }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Bold
                else if (tag == "b" || tag == "strong") { fontStyle |= Font::FontStyleFlags::bold;      doSetFont(); }
                else if (tag == "/b" || tag == "/strong") { fontStyle ^= Font::FontStyleFlags::bold;      doSetFont(); }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Underlined
                else if (tag == "u") { fontStyle |= Font::FontStyleFlags::underlined;  doSetFont(); }
                else if (tag == "/u") { fontStyle ^= Font::FontStyleFlags::underlined;  doSetFont(); }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Anchor
                else if (tag.startsWithIgnoreCase("a "))
                {
                    prev_fontColor = fontColor;
                    fontStyle = Font::FontStyleFlags::underlined;
                    doSetFont();
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
                    doSetFont();
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
                        doSetFont();
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
                        doSetFont();
                    }
                }
                else if (tag == "/font")
                {
                    fontFace = prev_fontFace;
                    fontSize = prev_fontSize;
                    doSetFont();

                    fontColor = prev_fontColor;
                    textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Font size modifiers "small" and "big"
                else if (tag == "small")
                {
                    prev_fontSize = fontSize;
                    fontSize *= 0.75f;
                    doSetFont();
                }
                else if (tag == "big")
                {
                    prev_fontSize = fontSize;
                    fontSize *= 1.25f;
                    doSetFont();
                }
                else if (tag == "/small" || tag == "/big")
                {
                    fontSize = prev_fontSize;
                    doSetFont();
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Lists
                else if (tag.startsWithIgnoreCase("ul"))
                {
                    lastListIsOrdered = false;
                    OrderedListCounter = 1;
                    lastIndentRange.setStart(charCounter + 1);
                }
                else if (tag.startsWithIgnoreCase("ol"))
                {
                    lastListIsOrdered = true;
                    OrderedListCounter = 1;
                    lastIndentRange.setStart(charCounter + 1);
                }
                else if (tag.startsWithIgnoreCase("li"))
                {
                    if (useImageIdents)
                    {
                        textEditor->insertTextAtCaret("\n"); charCounter++;
                    }
                    else
                    {
                        String indent = (lastListIsOrdered) ? "\n  " + String(OrderedListCounter) + ". " : "\n  - ";
                        textEditor->insertTextAtCaret(indent);
                        charCounter += indent.length();
                    }
                    OrderedListCounter++;
                }
                else if (tag.startsWithIgnoreCase("/ul") || tag.startsWithIgnoreCase("/ol"))
                {
                    if (useImageIdents)
                    {
                        lastIndentRange.setEnd(charCounter);
                        textEditor->setBounds(textEditor->getBounds().withHeight(textEditor->getTextHeight()));
                        auto b = textEditor->getTextBounds(lastIndentRange).getBounds();
                        auto* cmp = new ImageComponent(); ImageComponents.add(cmp);
                        cmp->setImage(textEditor->createComponentSnapshot(b));
                        cmp->setBounds(b.translated(25, 0));
                        textEditor->getChildComponent(0)->getChildComponent(0)->getChildComponent(0)->addAndMakeVisible(cmp);

                        textEditor->setHighlightedRegion(lastIndentRange); textEditor->cut(); charCounter = lastIndentRange.getStart();

                        String listSymbols;
                        for (int i = 1; i < OrderedListCounter; i++) listSymbols += (lastListIsOrdered) ? String(i) + ".\n" : " -\n";
                        textEditor->insertTextAtCaret(listSymbols);
                        charCounter += listSymbols.length();
                    }

                    // Add newline after unordered (or ordered) list
                    lastChar = '\n'; textEditor->insertTextAtCaret("\n"); charCounter++;
                    lastListIsOrdered = false;
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Headers
                else if (tag.startsWithIgnoreCase("h") && tag.containsAnyOf("1234"))
                {
                    prev_fontSize = fontSize;
                    auto sz = tag.getLastCharacters(1).getIntValue();
                    fontSize = 40 - sz * 4;
                    doSetFont();
                }
                else if (tag.startsWithIgnoreCase("/h") && tag.containsAnyOf("1234"))
                {
                    fontSize = prev_fontSize;
                    doSetFont();

                    // Add double newline after header text
                    lastChar = '\n'; textEditor->insertTextAtCaret("\n\n"); charCounter += 2;
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Paragraph
                else if (tag.startsWithIgnoreCase("p ") || tag == "p")
                {
                    // Add newline before paragraph
                    lastChar = '\n'; textEditor->insertTextAtCaret("\n"); charCounter++;

                    if (tag.containsIgnoreCase("style"))
                    {
                        int i1 = tag.indexOf("style=") + 6; int i2 = tag.indexOfChar(i1 + 1, '"');
                        auto styleString = tag.substring(i1, i2).replace("\"", "");
                        parseInlineStyle(styleString);
                    }
                }
                else if (tag == "/p")
                {
                    // Add newline after paragraph
                    lastChar = '\n'; textEditor->insertTextAtCaret("\n"); charCounter++;
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Span
                else if (tag.startsWithIgnoreCase("span "))
                {
                    if (tag.containsIgnoreCase("style"))
                    {
                        int i1 = tag.indexOf("style=") + 6; int i2 = tag.indexOfChar(i1 + 1, '"');
                        auto styleString = tag.substring(i1, i2).replace("\"", "");
                        parseInlineStyle(styleString);
                    }
                }
                else if (tag == "/span")
                {
                    fontSize = prev_fontSize;
                    fontColor = prev_fontColor;
                    fontFace = prev_fontFace;
                    fontStyle = Font::FontStyleFlags::plain;
                    doSetFont();

                    fontColor = prev_fontColor;
                    textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Pre-formatted
                else if (tag.startsWithIgnoreCase("pre"))
                {
                    renderPreFormatted = true;

                    fontStyle = Font::FontStyleFlags::plain;
                    prev_fontFace = fontFace;
                    prev_fontSize = fontSize;
                    fontSize = 12;
                    fontFace = Font::getDefaultMonospacedFontName();
                    doSetFont();

                    if (tag.containsIgnoreCase("style"))
                    {
                        int i1 = tag.indexOf("style=") + 6; int i2 = tag.indexOfChar(i1 + 1, '"');
                        auto styleString = tag.substring(i1, i2).replace("\"", "");
                        parseInlineStyle(styleString, false);
                    }
                }
                else if (tag == "/pre")
                {
                    renderPreFormatted = false;
                    fontSize = prev_fontSize;
                    fontFace = prev_fontFace;
                    doSetFont();

                    fontColor = prev_fontColor;
                    textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Image
                else if (tag.startsWithIgnoreCase("img "))
                {
                    int i1 = tag.indexOf("src=") + 5; int i2 = tag.indexOfChar(i1 + 1, '"');
                    auto ImgSrc = tag.substring(i1, i2).replace("\"", "");

                    MemoryBlock imgMemBlock;

                    // Search image in the resources
                    int size(0);
                    auto data = BinaryData::getNamedResource(ImgSrc.replace(".", "_").replace("-", "").toRawUTF8(), size);

                    // If image wasn't in the resource, attempt to load it from disk
                    if (size == 0)
                    {
                        auto ImgFile = File(File::getCurrentWorkingDirectory().getFullPathName() + File::getSeparatorString() + ImgSrc);
                        if (ImgFile.existsAsFile())
                        {
                            ImgFile.loadFileAsData(imgMemBlock);
                            size = imgMemBlock.getSize();
                            data = (const char*)imgMemBlock.getData();
                        }
                    }

                    if (size > 0)
                    {
                        ImagesInThisDocument.add(ImgSrc);

                        auto img = IMAGE_FROM_DATA_SIZE; //ImageCache::getFromMemory(data, size);
                        auto* cmp = ImageComponents.add(new ImageComponent(ImgSrc));
                        cmp->setImage(img);
                        int lastFontHeight = textEditor->getFont().getHeight();
                        int w = img.getWidth();
                        int h = img.getHeight();

                        // Resize image (set width, keep aspect ratio)
                        if (tag.containsIgnoreCase("width"))
                        {
                            int i1 = tag.indexOf("width=") + 6; int i2 = tag.indexOfChar(i1 + 1, '"');
                            auto val = tag.substring(i1, i2).replace("\"", "").getIntValue();
                            if (val > 0)
                            {
                                auto ratio = (float)h / (float)w;
                                w = val;
                                h = w * ratio;
                            }
                        }

                        // Set Image size and position
                        int x = textEditor->getLeftIndent();
                        int y = textEditor->getTextHeight() - lastFontHeight * 2;
                        cmp->setBounds(x, y, w, h);

                        // Now this is tricky! There's no way to get the viewport that contains the text in a TextEditor.
                        // This method digs into the component until reaching the viewport.
                        // Works with Juce 6.1.6 but may break if the class is modified in future versions of Juce.
                        textEditor->getChildComponent(0)->getChildComponent(0)->getChildComponent(0)->addAndMakeVisible(cmp);

                        // Now calculate the amount of break lines needed to move the text right below the image using the last font height
                        int shiftY = round((float)h / (float)lastFontHeight);
                        for (int i = 0; i < shiftY; i++)
                        {
                            lastChar = '\n'; textEditor->insertTextAtCaret("\n"); charCounter++;
                        }
                    }

                    else
                    {
                        String errMsg("[NOT FOUND: " + ImgSrc + "]\n");
                        textEditor->insertTextAtCaret(errMsg);
                        charCounter += errMsg.length();
                    }
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                tag.clear();
                continue;
            }

            // Skip multiple new lines
            if (lastChar == '\n' && s == '\n') continue;

            // Don't render white spaces at begin of new line
            if (lastChar == '\n' && s == ' ') continue;

            // Don't render multiple white spaces
            if (lastChar == ' ' && s == ' ') continue;

            // Replace new line with white space
            lastChar = (s == '\n') ? ' ' : s;

            // Add the plain characters to the output buffer
            output += String::charToString(lastChar);
            charCounter++;
        }

        // Print the remaining output buffer
        textEditor->setCaretPosition(charCounter);
        textEditor->insertTextAtCaret(output);

        totalTextHeight = textEditor->getTextHeight();
        DBG("Text Height: " << totalTextHeight);

        if (mobileStyle) resized();
    }

    // Pass a string to search for in the current document, or an empty string to clear search results
    bool searchAndHighlight(const String& keywords, bool restart = true)
    {
        // Reset search
        if (keywords.isEmpty())
        {
            lastSearchEndIndex = 0;
            lastSearchString.clear();
            textEditor->setHighlightedRegion({ 0,0 });
            return false;
        }

        lastSearchString = keywords;
        auto plainTextPage = textEditor->getText(); //DBG("plainTextPage = " << plainTextPage);

        if (plainTextPage.containsIgnoreCase(keywords))
        {
            auto start = plainTextPage.indexOfIgnoreCase(lastSearchEndIndex, keywords);

            // Reached end of results? Start searching from the beginning
            if (start < 0)
            {
                lastSearchEndIndex = 0;
                return restart ? GoToNextSearchResult() : false;
            }

            lastSearchEndIndex = start + keywords.length();

            textEditor->setHighlightedRegion({ start, lastSearchEndIndex });
            textEditor->grabKeyboardFocus();

            if (mobileStyle)
                mobileStyleViewPort.setViewPosition(0, textEditor->getCaretRectangle().getY());

            return true;
        }

        return false;
    }

    bool GoToNextSearchResult(bool restart = true)
    {
        return searchAndHighlight(lastSearchString, restart);
    }

    int GetScrollY()
    {
        if (mobileStyle)
            return mobileStyleViewPort.getViewPositionY();
        else
            return textEditor->getCaretRectangle().getY(); // Could not be accurate!
    }

    void SetScrollY(int y)
    {
        if (mobileStyle)
            mobileStyleViewPort.setViewPosition(0, y);
        // else ???
    }

    //==============================================================================

    Image getPageSnapshot()
    {
        return textEditor->createComponentSnapshot({ 0, 0, textEditor->getWidth() - mobileStyleViewPort.getScrollBarThickness(), totalTextHeight }, false);
    }

    // Export a PNG snapshot of the entire rendered page, works best in MobileStyle
    bool exportPageToImage(const URL& url)
    {
        if (!url.isLocalFile()) return false;

        return (url.getLocalFile().deleteFile()) ?
            PNGImageFormat().writeImageToStream(getPageSnapshot(), *url.createOutputStream())
            : false;
    }

    //==============================================================================

    void paintOverChildren(juce::Graphics& g) override
    {
        if (hoverLink)
        {
            hoverPosition.x = jlimit<int>(0, getWidth() - toolTipWidth, hoverPosition.x);
            hoverPosition.y = jlimit<int>(0, getHeight() - 25, hoverPosition.y);

            g.setFont(toolTipFont);
            g.setColour(Colours::beige);
            g.fillRoundedRectangle(hoverPosition.x, hoverPosition.y, toolTipWidth, 25, 5.f);
            g.setColour(Colours::black);
            g.drawText(hoverLinkText, hoverPosition.x, hoverPosition.y, toolTipWidth, 25, Justification::centred);
        }
    }

    void resized() override
    {
        textEditor->setBounds(0, 0, getWidth() - mobileStyleViewPort.getScrollBarThickness(), mobileStyle ? totalTextHeight : getHeight());

        if (transparentLayer != nullptr)
        {
            transparentLayer->setBounds(textEditor->getBounds());
            mobileStyleViewPort.setBounds(0, 0, getWidth(), getHeight());
        }
    }

    void mouseMove(const MouseEvent& event) override
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
                if (!l.url.startsWithIgnoreCase("http") || !showAnchorPopup) break;

                hoverLink = true;
                hoverLinkText = l.url;
                toolTipWidth = toolTipFont.getStringWidth(hoverLinkText) + 10;
                hoverPosition = event.getPosition() + juce::Point<int>(15, 15);
                repaint();
                break;
            }
        }
    }

    void mouseUp(const MouseEvent& event) override
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
                else
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

    String lastSearchString;
    bool useImageIdents = false;
    StringArray ImagesInThisDocument;

private:
    std::unique_ptr<TextEditor> textEditor;
    std::unique_ptr<Component> transparentLayer;
    Viewport mobileStyleViewPort;

    int charCounter, lastSearchEndIndex;
    String fontFace, prev_fontFace;
    float fontSize, prev_fontSize;
    int fontStyle = Font::FontStyleFlags::plain;
    Colour fontColor, prev_fontColor, linkColor = Colours::yellow;
    bool showAnchorPopup = true;
    bool mobileStyle = false;
    int totalTextHeight = 0;
    bool Comment = false;
    int OrderedListCounter = 0;
    bool lastListIsOrdered = false;

    bool hoverLink = false;
    String hoverLinkText = String();
    juce::Point<int> hoverPosition;
    Font toolTipFont = Font("Arial", 14.f, Font::FontStyleFlags::plain);
    int toolTipWidth = 100;

    OwnedArray<ImageComponent> ImageComponents;
    juce::Range<int> lastIndentRange;

    struct HyperLink
    {
        String url;
        Range<int> position;
    } tmpHL;
    Array<HyperLink> AllLinks;

    void doSetFont()
    {
        //textEditor->setFont(Font(fontFace, fontSize, fontStyle));

        auto theFont = Font(fontFace, fontSize, fontStyle);

        auto fontHeight = fontSize;
        int ff_size(0);
        auto ff_data = BinaryData::getNamedResource(fontFace.replace(".", "_").replace("-", "").toRawUTF8(), ff_size);
        if (ff_size > 0)
        {
            theFont = Font(Typeface::createSystemTypefaceFor(ff_data, ff_size));
            //#if JUCE_MAC || JUCE_IOS
            //fontHeight *= 0.75f; // Fonts look bigger on iOS and Mac OS!
            //#endif
        }

        theFont.setHeight(fontHeight);
        theFont.setStyleFlags(fontStyle);

        textEditor->setFont(theFont);
    }

    // Attempt to parse some basic inline CSS
    void parseInlineStyle(const String& inlineStyleString, bool remember = true)
    {
        if (remember)
        {
            prev_fontSize = fontSize;
            prev_fontColor = fontColor;
            prev_fontFace = fontFace;
        }

        auto styles = StringArray::fromTokens(inlineStyleString, ";", "'");
        for (auto style : styles)
        {
            auto pair = StringArray::fromTokens(style, ":", "'");
            if (pair.size() < 2) continue;
            auto key = pair[0].trim();
            auto val = pair[1].trim();
            //DBG("Key: " << key << " Value: " << val);

            if (key == "text-decoration")
            {
                if (val.contains("underline")) fontStyle |= Font::FontStyleFlags::underlined;
            }
            if (key == "font-weight")
            {
                if (val.contains("bold")) fontStyle |= Font::FontStyleFlags::bold;
            }
            if (key == "font-size")
            {
                if (val.contains("px")) fontSize = val.replace("px", "").getIntValue();
            }
            if (key == "font-family")
            {
                fontFace = val.replace("'", "");
            }
            if (key == "color")
            {
                auto col = val.replace("#", "").getHexValue32() + 0xFF000000;
                fontColor = Colour(col);
                textEditor->setColour(TextEditor::ColourIds::textColourId, fontColor);
            }
        }

        doSetFont();
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GSiHtmlTextEdit)
};
