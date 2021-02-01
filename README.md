# JuceHtmlTextEditor

This is an example program for a JUCE component that I wrote that can do some simple HTML 4 parsing of just the basic tags for formatting text and adding hyperlinks.
Since no component in the JUCE library can mix normal text and hyperlinks, the only way to do so was to mix multiple components and find a good solution to align them in a way 
that everything seems fluid like on a normal web page. Quite tricky, isn'it?

The JUCE component TextEditor can be multiline and supports changing font size, faces and colors, like an RTF document (or an HTML page!), but can't parse any markup language.
So I have added some functions to do the parsing and handle clickable links.

```html
Currently, the only tags that are correctly supported are:
- Bold <b>
- Italic <i>
- Underlined <u>
- Font <font> with properties size="" color="" and face=""
- Headers <h1> ... <h4>
- Unordered list <ul> and <li>
- Anchor <a>
- Break line <br>

Evething else between < and > will fail the parsing and will not be rendered as text, 
so also using comments between <!-- and --> will work.

Remember to use the quote marks for the value of a property. For example:
<a href="https://www.github.com">This will work</a>
<a href=https://www.github.com>This will not work</a>
(could be improved some way...)

Also, remember to close the tags with the corresponding slashed tag.

```
The example also shows how to handle the anchor tag to invoke program's internal functions such as opening a dialog window.
Two more custom classes are included, one to create buttons that can either be on/off switches or pushbuttons, and another to create a dialog window that doesn't need to be necessarily modal.

If you like this program, visit www.GenuineSoundware.com and try some of our software.
