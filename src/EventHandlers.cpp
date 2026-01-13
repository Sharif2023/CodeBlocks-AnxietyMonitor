#include "EventHandlers.h"
#include "DataCollector.h"

// Code::Blocks SDK compatibility layer
// In real build, these come from <sdk.h> and Code::Blocks headers
// We define minimal stubs here for compilation without full SDK

#ifndef CODEBLOCKS_SDK_INCLUDED
// Stub definitions when building without CB SDK
class wxScintillaEvent {};
class CodeBlocksEvent : public wxEvent {
public:
    CodeBlocksEvent(wxEventType type = wxEVT_NULL) : wxEvent(0, type) {}
    wxEvent* Clone() const override { return new CodeBlocksEvent(*this); }
    
    // Stub methods
    wxString GetEditor() const { return wxEmptyString; }
    int GetInt() const { return 0; }
    wxString GetString() const { return wxEmptyString; }
};
class cbEditor {
public:
    wxString GetFilename() const { return wxEmptyString; }
};
class cbProject {
public:
    wxString GetTitle() const { return wxEmptyString; }
};
#endif

namespace AnxietyMonitor {

EventHandlers::EventHandlers(DataCollector* collector)
    : m_collector(collector)
{
}

void EventHandlers::OnEditorUpdateUI(CodeBlocksEvent& event)
{
    // This event fires on any editor UI update (cursor move, selection, etc.)
    // We use it to track general activity
    if (m_collector && m_collector->IsRunning()) {
        // Just mark as activity - actual keystroke tracking in OnEditorCharAdded
    }
    
    // Always skip to allow other handlers to process
    event.Skip();
}

void EventHandlers::OnEditorCharAdded(CodeBlocksEvent& event)
{
    // Character was added to editor - this is our main keystroke capture
    if (m_collector && m_collector->IsRunning()) {
        // The event.GetInt() contains the character code in real CB SDK
        int charCode = event.GetInt();
        
        // Check for backspace (ASCII 8) or delete (127)
        bool isBackspace = (charCode == 8 || charCode == 127);
        
        m_collector->OnKeystroke(isBackspace);
    }
    
    event.Skip();
}

void EventHandlers::OnCompilerStart(CodeBlocksEvent& event)
{
    if (m_collector && m_collector->IsRunning()) {
        m_collector->OnCompileStart();
    }
    
    event.Skip();
}

void EventHandlers::OnCompilerFinished(CodeBlocksEvent& event)
{
    if (m_collector && m_collector->IsRunning()) {
        // In real CB SDK, we get error count from the event or CompilerManager
        // For now, use event.GetInt() which typically contains exit code
        int errorCount = event.GetInt();
        bool success = (errorCount == 0);
        
        // Warning count would come from CompilerManager::GetWarningCount()
        int warningCount = 0;
        
        m_collector->OnCompileEnd(errorCount, warningCount, success);
    }
    
    event.Skip();
}

void EventHandlers::OnAppStartupDone(CodeBlocksEvent& event)
{
    // App is fully initialized - good time to set up any remaining handlers
    event.Skip();
}

void EventHandlers::OnIdle(wxIdleEvent& event)
{
    // Called periodically when app is idle
    if (m_collector && m_collector->IsRunning()) {
        m_collector->OnIdleTick();
    }
    
    // Request more idle events if session is running
    if (m_collector && m_collector->IsRunning()) {
        event.RequestMore();
    }
    
    event.Skip();
}

void EventHandlers::OnEditorActivated(CodeBlocksEvent& event)
{
    if (m_collector) {
        m_collector->OnEditorFocusChange(true);
        
        // Track file change (tab switch)
        wxString currentFile = event.GetString();  // In CB SDK, this may vary
        if (!currentFile.IsEmpty() && currentFile != m_lastActiveFile) {
            m_collector->OnTabChange();
            m_collector->SetActiveFile(currentFile.ToStdString());
            m_lastActiveFile = currentFile;
        }
    }
    
    event.Skip();
}

void EventHandlers::OnEditorDeactivated(CodeBlocksEvent& event)
{
    if (m_collector) {
        m_collector->OnEditorFocusChange(false);
    }
    
    event.Skip();
}

void EventHandlers::OnProjectActivated(CodeBlocksEvent& event)
{
    if (m_collector) {
        wxString projectName = event.GetString();
        if (!projectName.IsEmpty()) {
            m_collector->SetActiveProject(projectName.ToStdString());
        }
    }
    
    event.Skip();
}

void EventHandlers::OnKeyDown(wxKeyEvent& event)
{
    if (m_collector && m_collector->IsRunning()) {
        int keyCode = event.GetKeyCode();
        
        // Detect Undo (Ctrl+Z)
        if (event.ControlDown() && keyCode == 'Z') {
            if (event.ShiftDown()) {
                // Ctrl+Shift+Z = Redo
                m_collector->OnRedo();
            } else {
                m_collector->OnUndo();
            }
        }
        // Detect Redo (Ctrl+Y)
        else if (event.ControlDown() && keyCode == 'Y') {
            m_collector->OnRedo();
        }
    }
    
    event.Skip();
}

} // namespace AnxietyMonitor
