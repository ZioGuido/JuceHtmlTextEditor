/*
  ==============================================================================

    Common_UI.h
    Created: 3 May 2020 1:10:06pm
    Author:  ziogu

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Class for a custom button component
class SquareButton : public Button
{
public:
    SquareButton(String name, int type = 0) : Button(name)
    {
        txt_on = txt_off = this->getButtonText();
        setType(type);
    }

    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // Make background color according to current status
        g.setColour(Colour(0xFF404244)); // Off
        if (status) g.setColour(Colour(0xFFFFFAFA)); // On
        if (type == 1) g.setColour(Colour(0xFF505570)); // Pushbutton

        g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 2);

        // Draw background image?
        if (img != Image())
            //g.drawImage(img, 0, 0, getWidth(), getHeight(), 0, 0, img.getWidth(), img.getHeight());
            g.drawImageAt(img, (getWidth() - img.getWidth()) / 2, 0);

        // Write label
        auto fontSize = jmin(14.0f, getHeight() * 0.6f);
        g.setFont(fontSize);
        g.setColour(Colours::white); 
        if (status) g.setColour(Colours::black);
        if (!isEnabled()) g.setColour(Colours::grey);
        g.drawFittedText(status ? txt_on : txt_off, 0, 0, getWidth(), getHeight(), Justification::centred, true);
    }

    /*void SetNewName(const String& name) 
    {
        this->setButtonText(name);
        repaint();
    }*/

    void setLabels(String off, String on = String())
    {
        txt_off = off;
        txt_on = on == String() ? txt_off : on;
        repaint();
    }

    void setImage(const Image& _img)
    {
        img = _img;
        repaint();
    }

    void setStatus(int s)
    {
        status = s;
        repaint();
    }

    int getStatus()
    {
        return status;
    }

    // 0 (defaul) = toggle; 1 = pushbutton
    void setType(int t)
    {
        type = t;
        if (type == 1) setMouseCursor(MouseCursor::PointingHandCursor);
    }

    void mouseUp(const MouseEvent& event) override
    {
        if (!isEnabled()) return;

        if (type == 0) status = !status;
        if (onClickCallback != nullptr) onClickCallback(event);
        repaint();
    }

    std::function<void(const MouseEvent& event)> onClickCallback;

private:
    int type = 0;
    int status = 1;
    Image img = Image();
    String txt_on, txt_off;
};

// Class for a custom dialog window
class GSiDialogWindow : public Component, KeyListener
{
public:
    enum AlertType { typeInfo = 0, typeInfoAutoClose, typeError, typeQuestion };
    static int constexpr animationTime = 150;
    static int constexpr AutoCloseTime = 2000;

    GSiDialogWindow()
    {
        setSize(400, 160);
        addKeyListener(this);
        setWantsKeyboardFocus(true);

        btnOK.reset(new SquareButton("OK", 1));
        btnOK->onClickCallback = [&](const MouseEvent&) { Close(); if (onButtonClick != nullptr) onButtonClick(true); };
        addAndMakeVisible(btnOK.get());

        btnCancel.reset(new SquareButton("Cancel", 1));
        btnCancel->setBounds(getWidth() - 120 - 40, getHeight() - 40, 120, 30);
        btnCancel->onClickCallback = [&](const MouseEvent&) { Close(); if (onButtonClick != nullptr) onButtonClick(false); };
        addChildComponent(btnCancel.get());
    }

    ~GSiDialogWindow()
    {
    }

    void paint(Graphics& g) override
    {
        // The background fill
        g.setColour(bkgColor);
        g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 5);

        // The title bar fill
        g.setColour(Colour(0xFFA0A0A0));
        g.fillRoundedRectangle(2, 2, getWidth() - 4, 30, 5);

        g.setColour(Colour(0xFFFAFAFA));

        // Title
        g.setFont(22.f);
        g.drawText(title, 6, 4, getWidth() - 12, 30, Justification::centredLeft);

        // Message
        g.setFont(16.f);
        g.drawText(message, 10, 40, getWidth() - 4, 80, Justification::centredLeft);
    }

    void Open(int type, String _title, String _message)
    {
        title = _title;
        message = _message;
        bkgColor = Colour(0xFA202020);

        switch (type)
        {
        case AlertType::typeInfo:
        case AlertType::typeInfoAutoClose:
        case AlertType::typeError:
            btnOK->setBounds((getWidth() - 120) / 2, getHeight() - 40, 120, 30);
            btnCancel->setVisible(false);

            if (type == AlertType::typeError)
                bkgColor = Colour(0xFAC02020);

            if (type == AlertType::typeInfoAutoClose)
                Timer::callAfterDelay(AutoCloseTime, [&] { Close(); });
            break;

        case AlertType::typeQuestion:
            btnOK->setBounds(40, getHeight() - 40, 120, 30);
            btnCancel->setVisible(true);
            break;
        }

        repaint();
        centreWithSize(getWidth(), getHeight());
        Desktop::getInstance().getAnimator().fadeIn(this, animationTime);
        toFront(true);
        //if (!isCurrentlyModal()) enterModalState(true, nullptr); // do you really need to make this modal?!
    }

    void Close()
    {
        Desktop::getInstance().getAnimator().fadeOut(this, animationTime);
        //exitModalState(0);
    }


    // Attach a lambda to perform button events
    std::function<void(bool)> onButtonClick;

private:
    std::unique_ptr<SquareButton> btnOK, btnCancel;
    String title, message;
    Colour bkgColor;
    ComponentDragger dragger;

    // Allow dragging this component
    void mouseDown(const MouseEvent& event) override
    {
        event.eventComponent->toFront(true);
        dragger.startDraggingComponent(this, event);
    }
    void mouseDrag(const MouseEvent& event) override
    {
        dragger.dragComponent(this, event, nullptr);
    }

    // Catch key presses
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override
    {
        // Catch the ESC key to hide this component
        if (key == KeyPress::escapeKey) Close();

        return false;
    }

    // ----------------------------------------------------------
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GSiDialogWindow)
};
