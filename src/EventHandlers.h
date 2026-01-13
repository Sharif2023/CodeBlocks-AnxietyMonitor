#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

#include <wx/event.h>

// Forward declarations for Code::Blocks SDK types
// These are simplified - actual CB SDK has more complex headers
class cbEditor;
class cbProject;
class CodeBlocksEvent;
class CompilerEvent;

namespace AnxietyMonitor {

class DataCollector;

/**
 * @class EventHandlers
 * @brief Handles Code::Blocks SDK events for metrics collection.
 * 
 * Connects to:
 * - cbEVT_EDITOR_UPDATE_UI: Typing activity
 * - cbEVT_EDITOR_CHAR_ADDED: Keystroke capture
 * - cbEVT_COMPILER_START: Compile start
 * - cbEVT_COMPILER_FINISHED: Compile end + errors
 * - cbEVT_APP_STARTUP_DONE: App ready
 * - cbEVT_IDLE: Idle detection
 * - cbEVT_EDITOR_ACTIVATED: Focus changes
 */
class EventHandlers {
public:
    EventHandlers(DataCollector* collector);
    ~EventHandlers() = default;
    
    // Code::Blocks event callbacks
    void OnEditorUpdateUI(CodeBlocksEvent& event);
    void OnEditorCharAdded(CodeBlocksEvent& event);
    void OnCompilerStart(CodeBlocksEvent& event);
    void OnCompilerFinished(CodeBlocksEvent& event);
    void OnAppStartupDone(CodeBlocksEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnEditorActivated(CodeBlocksEvent& event);
    void OnEditorDeactivated(CodeBlocksEvent& event);
    void OnProjectActivated(CodeBlocksEvent& event);
    
    // Additional handlers for specific key detection
    void OnKeyDown(wxKeyEvent& event);
    
private:
    DataCollector* m_collector;
    
    // Track last file for change detection
    wxString m_lastActiveFile;
};

} // namespace AnxietyMonitor

#endif // EVENT_HANDLERS_H
