#include <iostream>

// Mock wxWidgets headers to satisfy cbplugin.h dependencies
// We don't want to link against full wxWidgets just for this check
#define WXDLLIMPEXP_SDK 
#define WXDLLIMPEXP_CB_PLUGIN
#define cb_unused
class wxMenuBar {};
class wxMenu {};
class wxToolBar {};
class wxPanel {};
class wxWindow {};
class wxString {};
class wxEvent {};
class wxCommandEvent {};
class wxTimerEvent {};
class wxDC {};

// Define globals usually provided by other headers if needed
// But we'll try to include minimal necessary files

// Include the file containing the version
#include "cbplugin.h" 

int main() {
    std::cout << "MAJOR=" << PLUGIN_SDK_VERSION_MAJOR << std::endl;
    std::cout << "MINOR=" << PLUGIN_SDK_VERSION_MINOR << std::endl;
    std::cout << "RELEASE=" << PLUGIN_SDK_VERSION_RELEASE << std::endl;
    return 0;
}
