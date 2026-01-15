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
  wxEvent *Clone() const override { return new CodeBlocksEvent(*this); }

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

EventHandlers::EventHandlers(DataCollector *collector)
    : m_collector(collector) {}

void EventHandlers::OnEditorUpdateUI(CodeBlocksEvent &event) {
  // DISABLED for debugging freeze
  /*
  if (m_collector && m_collector->IsRunning()) {
  }
  */
  event.Skip();
}

void EventHandlers::OnEditorCharAdded(CodeBlocksEvent &event) {
  // Character was added to editor - this is our main keystroke capture
  if (m_collector && m_collector->IsRunning()) {
    int charCode = event.GetInt();
    bool isBackspace = (charCode == 8 || charCode == 127);
    m_collector->OnKeystroke(isBackspace);
  }
  event.Skip();
}

void EventHandlers::OnCompilerStart(CodeBlocksEvent &event) {
  // DISABLED: Suspected deadlock source
  /*
  if (m_collector && m_collector->IsRunning()) {
      m_collector->OnCompileStart();
  }
  */
  event.Skip();
}

void EventHandlers::OnCompilerFinished(CodeBlocksEvent &event) {
  // DISABLED: Suspected deadlock source
  /*
  if (m_collector && m_collector->IsRunning()) {
      int errorCount = event.GetInt();
      bool success = (errorCount == 0);
      int warningCount = 0;
      m_collector->OnCompileEnd(errorCount, warningCount, success);
  }
  */
  event.Skip();
}

void EventHandlers::OnAppStartupDone(CodeBlocksEvent &event) { event.Skip(); }

void EventHandlers::OnIdle(wxIdleEvent &event) {
  // DISABLED: Suspected freeze source
  /*
  if (m_collector && m_collector->IsRunning()) {
      m_collector->OnIdleTick();
  }
  */
  event.Skip();
}

void EventHandlers::OnEditorActivated(CodeBlocksEvent &event) {
  if (m_collector) {
    m_collector->OnEditorFocusChange(true);

    // Track file change (tab switch)
    wxString currentFile = event.GetString(); // In CB SDK, this may vary
    if (!currentFile.IsEmpty() && currentFile != m_lastActiveFile) {
      m_collector->OnTabChange();
      m_collector->SetActiveFile(currentFile.ToStdString());
      m_lastActiveFile = currentFile;
    }
  }

  event.Skip();
}

void EventHandlers::OnEditorDeactivated(CodeBlocksEvent &event) {
  if (m_collector) {
    m_collector->OnEditorFocusChange(false);
  }

  event.Skip();
}

void EventHandlers::OnProjectActivated(CodeBlocksEvent &event) {
  if (m_collector) {
    wxString projectName = event.GetString();
    if (!projectName.IsEmpty()) {
      m_collector->SetActiveProject(projectName.ToStdString());
    }
  }

  event.Skip();
}

void EventHandlers::OnKeyDown(wxKeyEvent &event) {
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
