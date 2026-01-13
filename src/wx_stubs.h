#ifndef WX_STUBS_H
#define WX_STUBS_H

/**
 * @file wx_stubs.h
 * @brief Minimal wxWidgets stubs for standalone compilation without wxWidgets SDK.
 * 
 * This allows the code to compile and unit tests to run even when wxWidgets
 * is not installed. For full plugin functionality, wxWidgets 3.0+ is required.
 */

#ifdef STANDALONE_BUILD

#include <string>
#include <functional>

// Basic wxWidgets type stubs
typedef int wxWindowID;
typedef unsigned long wxEventType;

#define wxID_ANY (-1)
#define wxID_HIGHEST 5999
#define wxEVT_NULL 0
#define wxDefaultPosition wxPoint()
#define wxDefaultSize wxSize()
#define wxEmptyString std::string()

// Forward declarations
class wxWindow;
class wxEvent;
class wxCommandEvent;
class wxTimerEvent;
class wxIdleEvent;
class wxKeyEvent;
class wxToolBar;
class wxMenuBar;
class wxMenu;
class wxPanel;
class wxColour;
class wxString;
class wxTimer;

// wxString stub
class wxString : public std::string {
public:
    wxString() : std::string() {}
    wxString(const char* s) : std::string(s ? s : "") {}
    wxString(const std::string& s) : std::string(s) {}
    
    std::string ToStdString() const { return *this; }
    bool IsEmpty() const { return empty(); }
    
    static wxString Format(const char* fmt, ...) { return wxString(fmt); }
};

// wxPoint stub
struct wxPoint {
    int x, y;
    wxPoint(int _x = 0, int _y = 0) : x(_x), y(_y) {}
};

// wxSize stub
struct wxSize {
    int width, height;
    wxSize(int w = -1, int h = -1) : width(w), height(h) {}
};

// wxColour stub
class wxColour {
public:
    wxColour(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0) 
        : m_r(r), m_g(g), m_b(b) {}
private:
    unsigned char m_r, m_g, m_b;
};

// wxEvent stub
class wxEvent {
public:
    wxEvent(int id = 0, wxEventType type = wxEVT_NULL) : m_id(id), m_type(type) {}
    virtual ~wxEvent() {}
    virtual wxEvent* Clone() const { return new wxEvent(*this); }
    void Skip(bool skip = true) { m_skip = skip; }
    int GetId() const { return m_id; }
private:
    int m_id;
    wxEventType m_type;
    bool m_skip = false;
};

// wxCommandEvent stub
class wxCommandEvent : public wxEvent {
public:
    wxCommandEvent(wxEventType type = wxEVT_NULL, int id = 0) : wxEvent(id, type) {}
};

// wxTimerEvent stub
class wxTimerEvent : public wxEvent {
public:
    wxTimerEvent() : wxEvent() {}
};

// wxIdleEvent stub
class wxIdleEvent : public wxEvent {
public:
    void RequestMore(bool needMore = true) { (void)needMore; }
};

// wxKeyEvent stub
class wxKeyEvent : public wxEvent {
public:
    int GetKeyCode() const { return 0; }
    bool ControlDown() const { return false; }
    bool ShiftDown() const { return false; }
};

// wxEvtHandler stub
class wxEvtHandler {
public:
    virtual ~wxEvtHandler() {}
    template<typename T>
    void Bind(wxEventType, T, wxEvtHandler*, int) {}
};

// wxWindow stub
class wxWindow : public wxEvtHandler {
public:
    wxWindow(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY) 
        : m_parent(parent), m_id(id) {}
    virtual ~wxWindow() {}
    
    void SetBackgroundColour(const wxColour&) {}
    void SetForegroundColour(const wxColour&) {}
    void Show(bool show = true) { (void)show; }
    void Hide() {}
    void Refresh() {}
    void Layout() {}
    
protected:
    wxWindow* m_parent;
    wxWindowID m_id;
};

// wxPanel stub
class wxPanel : public wxWindow {
public:
    wxPanel(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize)
        : wxWindow(parent, id) { (void)pos; (void)size; }
};

// wxTimer stub
class wxTimer {
public:
    wxTimer(wxEvtHandler* owner = nullptr, int id = wxID_ANY) 
        : m_owner(owner), m_id(id), m_running(false) {}
    
    void Start(int milliseconds, bool oneShot = false) { 
        (void)milliseconds; (void)oneShot; m_running = true; 
    }
    void Stop() { m_running = false; }
    bool IsRunning() const { return m_running; }
    
private:
    wxEvtHandler* m_owner;
    int m_id;
    bool m_running;
};

// wxToolBar stub
class wxToolBar : public wxWindow {
public:
    void AddTool(int, const char*, const wxString&, const char*) {}
    void AddSeparator() {}
    void EnableTool(int, bool) {}
    void Realize() {}
};

// wxMenu/wxMenuBar stubs
class wxMenu {
public:
    void Append(int, const char*, const char* = nullptr) {}
    void AppendSeparator() {}
};

class wxMenuBar {
public:
    int FindMenu(const char*) { return -1; }
    void Insert(int, wxMenu*, const char*) {}
    void Append(wxMenu*, const char*) {}
};

// wxMessageBox stub
inline int wxMessageBox(const char*, const char*, long = 0, wxWindow* = nullptr) { return 0; }

// wxLogMessage/wxLogError stubs
inline void wxLogMessage(const char*, ...) {}
inline void wxLogError(const char*, ...) {}

// wxCopyFile stub
inline bool wxCopyFile(const wxString&, const wxString&) { return true; }

// Event table macros
#define wxDECLARE_EVENT_TABLE()
#define wxBEGIN_EVENT_TABLE(a, b)
#define wxEND_EVENT_TABLE()
#define EVT_TIMER(id, func)
#define EVT_BUTTON(id, func)
#define EVT_MENU(id, func)
#define wxEVT_TOOL 0

// ArtProvider stub
class wxArtProvider {
public:
    static wxString GetBitmap(const char*, const char*) { return wxString(); }
};

#define wxART_GO_FORWARD "go-forward"
#define wxART_TICK_MARK "tick"
#define wxART_FILE_SAVE "save"
#define wxART_HELP_SETTINGS "settings"
#define wxART_TOOLBAR "toolbar"

// Sizers (minimal stubs)
class wxSizer {
public:
    virtual ~wxSizer() {}
    void Add(wxWindow*, int = 0, int = 0, int = 0) {}
    void Add(wxSizer*, int = 0, int = 0, int = 0) {}
    void AddSpacer(int) {}
};

class wxBoxSizer : public wxSizer {
public:
    wxBoxSizer(int) {}
};

class wxFlexGridSizer : public wxSizer {
public:
    wxFlexGridSizer(int, int = 0, int = 0) {}
    void AddGrowableCol(int, int = 1) {}
};

#define wxVERTICAL 0
#define wxHORIZONTAL 1
#define wxALL 0
#define wxEXPAND 0
#define wxALIGN_CENTER 0
#define wxALIGN_CENTER_VERTICAL 0
#define wxLEFT 0
#define wxRIGHT 0
#define wxTOP 0
#define wxBOTTOM 0

// Static controls
class wxStaticText : public wxWindow {
public:
    wxStaticText(wxWindow*, wxWindowID, const char*) {}
    void SetLabel(const wxString&) {}
    wxFont GetFont() const { return wxFont(); }
    void SetFont(const wxFont&) {}
    void Wrap(int) {}
};

class wxStaticLine : public wxWindow {
public:
    wxStaticLine(wxWindow*, wxWindowID) {}
};

class wxListBox : public wxWindow {
public:
    wxListBox(wxWindow*, wxWindowID, const wxPoint& = wxDefaultPosition,
              const wxSize& = wxDefaultSize, int = 0, 
              const wxString* = nullptr, long = 0) {}
    void Insert(const std::string&, int) {}
    void Delete(int) {}
    void Clear() {}
    size_t GetCount() const { return 0; }
};

class wxGauge : public wxWindow {
public:
    wxGauge(wxWindow*, wxWindowID, int, const wxPoint& = wxDefaultPosition,
            const wxSize& = wxDefaultSize, long = 0) {}
    void SetValue(int) {}
};

#define wxGA_HORIZONTAL 0
#define wxGA_SMOOTH 0
#define wxLB_SINGLE 0
#define wxBORDER_NONE 0

// Font
class wxFont {
public:
    void SetPointSize(int) {}
    void SetWeight(int) {}
};

#define wxFONTWEIGHT_BOLD 0

// DateTime
class wxDateTime {
public:
    static wxDateTime Now() { return wxDateTime(); }
    wxString Format(const char*) const { return wxString(); }
};

// FileDialog
#define wxFD_SAVE 0
#define wxFD_OVERWRITE_PROMPT 0
#define wxID_CANCEL 0
#define wxOK 0
#define wxICON_INFORMATION 0
#define wxICON_WARNING 0
#define wxICON_EXCLAMATION 0
#define wxICON_ERROR 0
#define wxNOT_FOUND (-1)

class wxFileDialog {
public:
    wxFileDialog(wxWindow*, const char*, const wxString& = wxEmptyString,
                 const wxString& = wxEmptyString, const char* = nullptr, long = 0) {}
    int ShowModal() { return wxID_CANCEL; }
    wxString GetPath() const { return wxString(); }
};

#endif // STANDALONE_BUILD

#endif // WX_STUBS_H
