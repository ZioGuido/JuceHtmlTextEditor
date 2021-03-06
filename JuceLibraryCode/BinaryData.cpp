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
"<u><b>This text is   bold   and   underlined.</b></u>\r\n"
"<br>\r\n"
"<font color=\"#FF2020\">This text is red and contains&nbsp;&nbsp;&nbsp; multiple&nbsp;&nbsp;&nbsp; &quot;whitespaces&quot;.</font><br>\r\n"
"<font size=\"32\" color=\"#20FF20\">This text is big and &laquo;green&raquo;</font>\r\n"
"<br>\r\n"
"<a href=\"https://www.juce.com\">This is a    hyperlink to    www.juce.com</a>\r\n"
"<br>\r\n"
"<br>\r\n"
"<h3>This is a header</h3>\r\n"
"<br>\r\n"
"<font face=\"Times New Roman\">This line in Times New Roman introduces an     unordered list</font>\r\n"
"<ul>\r\n"
"<li>Item # 1<li>Item # 2\r\n"
"<li>Item # 3\r\n"
"</ul>\r\n"
"<br>\r\n"
"<a href=\"page2.htm\">This link calls an internal function</a> that will load a new HTML page.\r\n"
"<br>\r\n"
"<br>\r\n"
"<a href=\"#alert=Hello world.\">This link will open a dialog window</a>.\r\n"
"<br>\r\n"
"\r\n"
"<pre>\r\n"
"// This is a pre-formatted text, useful for rendering source code\r\n"
"int doSum(int a, int b)\r\n"
"{\r\n"
"    return a + b;\r\n"
"}\r\n"
"</pre>\r\n"
"<br>\r\n"
"\r\n"
"\r\n"
"If this program has been compiled\r\n"
"under <span style=\"text-decoration: underline; color: #CC0000; font-weight: bold; font-size: 25px; font-family: 'Comic Sans MS'\">Windows</span> in _DEBUG mode,\r\n"
"you can modify this HTML page\r\n"
"and press F5 to reload the file\r\n"
"and see the results immediately\r\n"
"without having to recompile.\r\n"
"\r\n"
"\r\n"
"<!--\r\n"
"This is a comment.\r\n"
"Multiple consecutive white spaces or new lines \r\n"
"have been inserted to check that they're properly rendered.\r\n"
"This comment will not appear in the rendered page.\r\n"
"-->\r\n";

const char* page1_htm = (const char*) temp_binary_data_0;

//================== page2.htm ==================
static const unsigned char temp_binary_data_1[] =
"\r\n"
"<h2>Welcome to the HtmlTextEditor example program</h2>\r\n"
"\r\n"
"<h1>This is page 2</h1>\r\n"
"\r\n"
"<br>\r\n"
"<a href=\"page1.htm\">Return to page 1</a>.\r\n"
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
        case 0x321d22a4:  numBytes = 1606; return page1_htm;
        case 0x322b3a25:  numBytes = 309; return page2_htm;
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
