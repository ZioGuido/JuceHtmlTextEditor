/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== page1.htm ==================
static const unsigned char temp_binary_data_0[] =
"\r\n"
"<h2>Welcome to the HtmlTextEditor example program</h2>\r\n"
"\r\n"
"This is normal text.\r\n"
"<br>\r\n"
"<b>This text is bold.</b>\r\n"
"<br>\r\n"
"<i>This text is italic.</i>\r\n"
"<br>\r\n"
"<u>This text is underlined.</u>\r\n"
"<br>\r\n"
"<u><b>This text is bold and underlined.</b></u>\r\n"
"<br>\r\n"
"<font color=\"#FF2020\">This text is red</font>\r\n"
"<br>\r\n"
"<font size=\"32\" color=\"#20FF20\">This text is big and green</font>\r\n"
"<br>\r\n"
"<a href=\"https://www.juce.com\">This is an hyperlink to www.juce.com</a>\r\n"
"<br>\r\n"
"<br>\r\n"
"<h3>This is an header</h3>\r\n"
"<br>\r\n"
"<font face=\"Times New Roman\">This line in Times New Roman introduces an unordered list</font>\r\n"
"<ul>\r\n"
"<li>Item # 1\r\n"
"<li>Item # 2\r\n"
"<li>Item # 3\r\n"
"</ul>\r\n"
"<br>\r\n"
"<a href=\"#page2\">This link calls an internal function</a> that will load a new HTML page.\r\n"
"<br>\r\n"
"<br>\r\n"
"<a href=\"#alert=Hello world.\">This link will open a dialog window</a>.\r\n"
"<br>\r\n"
"Some final plain text.\r\n";

const char* page1_htm = (const char*) temp_binary_data_0;

//================== page2.htm ==================
static const unsigned char temp_binary_data_1[] =
"\r\n"
"<h2>Welcome to the HtmlTextEditor example program</h2>\r\n"
"\r\n"
"<h1>This is page 2</h1>\r\n"
"\r\n"
"<br>\r\n"
"<a href=\"#page1\">Return to page 1</a>.\r\n"
"<br>\r\n"
"<br>\r\n"
"If you like this program, visit <a href=\"https://www.genuinesoundware.com\">www.GenuineSoundware.com</a> and share some thought with us.\r\n"
"<br>\r\n"
"<br>\r\n"
"Thank you.\r\n";

const char* page2_htm = (const char*) temp_binary_data_1;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x321d22a4:  numBytes = 856; return page1_htm;
        case 0x322b3a25:  numBytes = 306; return page2_htm;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "page1_htm",
    "page2_htm"
};

const char* originalFilenames[] =
{
    "page1.htm",
    "page2.htm"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

}
