#include "AnxietyMonitor.h"
#include "DataCollector.h"
#include "CSVWriter.h"
#include "AnxietyScorer.h"
#include "UIComponents.h"
#include "EventHandlers.h"

#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/log.h>

#include <chrono>
#include <iomanip>
#include <sstream>

using namespace AnxietyMonitor;

// ============================================================================
// Event IDs
// ============================================================================
enum {
    ID_ANXIETY_START = wxID_HIGHEST + 1000,
    ID_ANXIETY_PAUSE,
    ID_ANXIETY_EXPORT,
    ID_ANXIETY_SETTINGS,
    ID_ANXIETY_TIMER,
    ID_MENU_START,
    ID_MENU_PAUSE,
    ID_MENU_END,
    ID_MENU_SHOW_PANEL
};

// ============================================================================
// Event Table
// ============================================================================
wxBEGIN_EVENT_TABLE(AnxietyMonitorPlugin, cbPlugin)
    EVT_TIMER(ID_ANXIETY_TIMER, AnxietyMonitorPlugin::OnTimerUpdate)
    EVT_BUTTON(ID_ANXIETY_START, AnxietyMonitorPlugin::OnStartButton)
    EVT_BUTTON(ID_ANXIETY_PAUSE, AnxietyMonitorPlugin::OnPauseButton)
    EVT_BUTTON(ID_ANXIETY_EXPORT, AnxietyMonitorPlugin::OnExportButton)
    EVT_BUTTON(ID_ANXIETY_SETTINGS, AnxietyMonitorPlugin::OnSettingsButton)
    EVT_MENU(ID_MENU_START, AnxietyMonitorPlugin::OnMenuStart)
    EVT_MENU(ID_MENU_PAUSE, AnxietyMonitorPlugin::OnMenuPause)
    EVT_MENU(ID_MENU_END, AnxietyMonitorPlugin::OnMenuEnd)
    EVT_MENU(ID_MENU_SHOW_PANEL, AnxietyMonitorPlugin::OnMenuShowPanel)
wxEND_EVENT_TABLE()

// ============================================================================
// Plugin Registration (Code::Blocks)
// ============================================================================
#ifdef CODEBLOCKS_SDK_INCLUDED
namespace {
    PluginRegistrant<AnxietyMonitorPlugin> reg("AnxietyMonitor");
}
#endif

// ============================================================================
// Constructor / Destructor
// ============================================================================

AnxietyMonitorPlugin::AnxietyMonitorPlugin()
    : m_panel(nullptr)
    , m_toolBar(nullptr)
    , m_startButtonId(ID_ANXIETY_START)
    , m_pauseButtonId(ID_ANXIETY_PAUSE)
    , m_exportButtonId(ID_ANXIETY_EXPORT)
    , m_settingsButtonId(ID_ANXIETY_SETTINGS)
    , m_updateTimer(this, ID_ANXIETY_TIMER)
    , m_sessionState(SessionState::STOPPED)
    , m_isInitialized(false)
{
    // Set default plugin info (for CB SDK)
    // In real plugin, this uses PluginInfo structure
}

AnxietyMonitorPlugin::~AnxietyMonitorPlugin()
{
    // Ensure cleanup happens even if OnRelease wasn't called
    if (m_isInitialized) {
        OnRelease(true);
    }
}

// ============================================================================
// Plugin Lifecycle
// ============================================================================

void AnxietyMonitorPlugin::OnAttach()
{
    wxLogMessage("AnxietyMonitor: Plugin attaching...");
    
    try {
        InitializeComponents();
        RegisterEventHandlers();
        SetupStatusBar();
        
        m_isInitialized = true;
        
        wxLogMessage("AnxietyMonitor: Plugin attached successfully.");
    }
    catch (const std::exception& e) {
        wxLogError("AnxietyMonitor: Failed to initialize - %s", e.what());
    }
}

void AnxietyMonitorPlugin::OnRelease(bool appShutDown)
{
    wxLogMessage("AnxietyMonitor: Plugin releasing... (shutdown=%d)", appShutDown);
    
    // CRITICAL: Auto-save any active session before exit
    if (m_sessionState != SessionState::STOPPED) {
        wxLogMessage("AnxietyMonitor: Auto-saving session before exit...");
        ForceSave();
        
        // End the session cleanly
        if (m_csvWriter) {
            m_csvWriter->EndSession();
        }
        if (m_dataCollector) {
            m_dataCollector->EndSession();
        }
    }
    
    // Stop the timer
    if (m_updateTimer.IsRunning()) {
        m_updateTimer.Stop();
    }
    
    // Unregister event handlers
    UnregisterEventHandlers();
    
    // Components will be cleaned up by unique_ptr destructors
    m_isInitialized = false;
    
    wxLogMessage("AnxietyMonitor: Plugin released.");
}

int AnxietyMonitorPlugin::GetConfigurationGroup() const
{
    return 0;  // No special configuration group
}

// ============================================================================
// Initialization
// ============================================================================

void AnxietyMonitorPlugin::InitializeComponents()
{
    // Create core components
    m_dataCollector = std::make_unique<DataCollector>();
    m_csvWriter = std::make_unique<CSVWriter>();
    m_scorer = std::make_unique<AnxietyScorer>();
    m_statusBarManager = std::make_unique<StatusBarManager>();
    
    // Create event handlers with reference to data collector
    m_eventHandlers = std::make_unique<EventHandlers>(m_dataCollector.get());
    
    // Apply default settings
    m_settings = PluginSettings();  // Uses defaults
    
    // Set CSV output directory
    m_csvWriter->SetOutputDirectory(CSVWriter::GetDefaultOutputDirectory());
    
    wxLogMessage("AnxietyMonitor: Components initialized. CSV dir: %s", 
                 CSVWriter::GetDefaultOutputDirectory().c_str());
}

void AnxietyMonitorPlugin::RegisterEventHandlers()
{
    // In real Code::Blocks plugin, we'd register with Manager::Get()
    // This connects our handlers to Code::Blocks events
    
#ifdef CODEBLOCKS_SDK_INCLUDED
    Manager* mgr = Manager::Get();
    if (mgr) {
        mgr->RegisterEventSink(cbEVT_EDITOR_UPDATE_UI, 
            new cbEventFunctor<AnxietyMonitorPlugin, CodeBlocksEvent>(
                this, &AnxietyMonitorPlugin::OnEditorEvent));
        mgr->RegisterEventSink(cbEVT_COMPILER_STARTED,
            new cbEventFunctor<AnxietyMonitorPlugin, CodeBlocksEvent>(
                this, &AnxietyMonitorPlugin::OnCompilerStart));
        mgr->RegisterEventSink(cbEVT_COMPILER_FINISHED,
            new cbEventFunctor<AnxietyMonitorPlugin, CodeBlocksEvent>(
                this, &AnxietyMonitorPlugin::OnCompilerFinished));
        // ... more event registrations
    }
#endif
    
    wxLogMessage("AnxietyMonitor: Event handlers registered.");
}

void AnxietyMonitorPlugin::UnregisterEventHandlers()
{
#ifdef CODEBLOCKS_SDK_INCLUDED
    Manager* mgr = Manager::Get();
    if (mgr) {
        mgr->RemoveAllEventSinksFor(this);
    }
#endif
}

void AnxietyMonitorPlugin::SetupStatusBar()
{
    // Status bar setup in Code::Blocks
    // We don't create our own - we update an existing field
    wxLogMessage("AnxietyMonitor: Status bar ready.");
}

// ============================================================================
// Toolbar
// ============================================================================

bool AnxietyMonitorPlugin::BuildToolBar(wxToolBar* toolBar)
{
    if (!toolBar) {
        return false;
    }
    
    m_toolBar = toolBar;
    
    // Add separator before our tools
    toolBar->AddSeparator();
    
    // Start button
    toolBar->AddTool(ID_ANXIETY_START, "Start",
                     wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR),
                     "Start Anxiety Monitoring Session");
    
    // Pause button
    toolBar->AddTool(ID_ANXIETY_PAUSE, "Pause",
                     wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_TOOLBAR),
                     "Pause/Resume Monitoring");
    
    // Export button
    toolBar->AddTool(ID_ANXIETY_EXPORT, "Export",
                     wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR),
                     "Export Session Data");
    
    // Settings button
    toolBar->AddTool(ID_ANXIETY_SETTINGS, "Settings",
                     wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_TOOLBAR),
                     "Anxiety Monitor Settings");
    
    // Connect events
    toolBar->Bind(wxEVT_TOOL, &AnxietyMonitorPlugin::OnStartButton, this, ID_ANXIETY_START);
    toolBar->Bind(wxEVT_TOOL, &AnxietyMonitorPlugin::OnPauseButton, this, ID_ANXIETY_PAUSE);
    toolBar->Bind(wxEVT_TOOL, &AnxietyMonitorPlugin::OnExportButton, this, ID_ANXIETY_EXPORT);
    toolBar->Bind(wxEVT_TOOL, &AnxietyMonitorPlugin::OnSettingsButton, this, ID_ANXIETY_SETTINGS);
    
    // Initial state - only Start enabled
    toolBar->EnableTool(ID_ANXIETY_PAUSE, false);
    toolBar->EnableTool(ID_ANXIETY_EXPORT, false);
    
    toolBar->Realize();
    
    wxLogMessage("AnxietyMonitor: Toolbar built with 4 buttons.");
    return true;
}

void AnxietyMonitorPlugin::BuildMenu(wxMenuBar* menuBar)
{
    if (!menuBar) return;
    
    // Create our menu
    wxMenu* anxietyMenu = new wxMenu();
    anxietyMenu->Append(ID_MENU_START, "&Start Session\tCtrl+Shift+S", 
                        "Start anxiety monitoring");
    anxietyMenu->Append(ID_MENU_PAUSE, "&Pause/Resume\tCtrl+Shift+P",
                        "Pause or resume monitoring");
    anxietyMenu->Append(ID_MENU_END, "&End Session\tCtrl+Shift+E",
                        "End current session and save data");
    anxietyMenu->AppendSeparator();
    anxietyMenu->Append(ID_MENU_SHOW_PANEL, "Show &Panel",
                        "Show/hide the anxiety monitor panel");
    
    // Insert before Help menu (typically last)
    int helpMenuPos = menuBar->FindMenu("Help");
    if (helpMenuPos != wxNOT_FOUND) {
        menuBar->Insert(helpMenuPos, anxietyMenu, "&Anxiety Monitor");
    } else {
        menuBar->Append(anxietyMenu, "&Anxiety Monitor");
    }
}

void AnxietyMonitorPlugin::BuildModuleMenu(const int type, wxMenu* menu)
{
    // We don't add to context menus to stay non-intrusive
    (void)type;
    (void)menu;
}

// ============================================================================
// Session Control
// ============================================================================

void AnxietyMonitorPlugin::StartSession()
{
    if (m_sessionState == SessionState::RUNNING) {
        return;  // Already running
    }
    
    // Generate session ID
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y%m%d_%H%M%S");
    m_currentSessionId = "session_" + oss.str();
    
    // Start CSV file
    if (!m_csvWriter->StartSession(m_currentSessionId)) {
        wxLogError("AnxietyMonitor: Failed to create session file.");
        return;
    }
    
    // Start data collection
    m_dataCollector->StartSession();
    
    // Start update timer (30 seconds)
    m_updateTimer.Start(UPDATE_INTERVAL_MS);
    
    m_sessionState = SessionState::RUNNING;
    
    // Update UI
    UpdateUI();
    
    wxLogMessage("AnxietyMonitor: Session started - %s", m_currentSessionId.c_str());
    wxLogMessage("AnxietyMonitor: CSV file: %s", m_csvWriter->GetCurrentFilePath().c_str());
}

void AnxietyMonitorPlugin::PauseSession()
{
    if (m_sessionState == SessionState::RUNNING) {
        // Pause - flush data first
        ForceSave();
        m_dataCollector->PauseSession();
        m_sessionState = SessionState::PAUSED;
        
        // Keep timer running for UI updates, but data collection is paused
        
        wxLogMessage("AnxietyMonitor: Session paused.");
    }
    else if (m_sessionState == SessionState::PAUSED) {
        // Resume
        m_dataCollector->ResumeSession();
        m_sessionState = SessionState::RUNNING;
        
        wxLogMessage("AnxietyMonitor: Session resumed.");
    }
    
    UpdateUI();
}

void AnxietyMonitorPlugin::ResumeSession()
{
    PauseSession();  // Toggle behavior
}

void AnxietyMonitorPlugin::EndSession()
{
    if (m_sessionState == SessionState::STOPPED) {
        return;  // Nothing to end
    }
    
    // CRITICAL: Force save before ending
    ForceSave();
    
    // Stop timer
    m_updateTimer.Stop();
    
    // End data collection and CSV
    m_dataCollector->EndSession();
    m_csvWriter->EndSession();
    
    m_sessionState = SessionState::STOPPED;
    
    // Update UI
    UpdateUI();
    
    wxLogMessage("AnxietyMonitor: Session ended. Total rows: %d", 
                 m_csvWriter->GetRowsWritten());
    
    // Show completion message (non-intrusive - just a status update)
    // Don't show popup unless specifically requested
}

void AnxietyMonitorPlugin::ExportSession()
{
    if (m_csvWriter->GetCurrentFilePath().empty()) {
        wxMessageBox("No active session to export.", "Anxiety Monitor",
                     wxOK | wxICON_INFORMATION);
        return;
    }
    
    // Flush current data
    m_csvWriter->Flush();
    
    // Show file dialog
    wxFileDialog saveDialog(nullptr, "Export Session Data",
                           wxEmptyString, wxEmptyString,
                           "CSV files (*.csv)|*.csv",
                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    
    if (saveDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    
    // Copy file to chosen location
    wxString srcPath = m_csvWriter->GetCurrentFilePath();
    wxString dstPath = saveDialog.GetPath();
    
    if (wxCopyFile(srcPath, dstPath)) {
        wxMessageBox(wxString::Format("Session exported to:\n%s", dstPath),
                     "Export Complete", wxOK | wxICON_INFORMATION);
    } else {
        wxMessageBox("Failed to export session file.", "Export Error",
                     wxOK | wxICON_ERROR);
    }
}

void AnxietyMonitorPlugin::ShowSettings()
{
    // Simple settings dialog (keeping it non-intrusive)
    wxMessageBox(
        "Anxiety Monitor Settings\n\n"
        "Current Configuration:\n"
        "- Update interval: 30 seconds\n"
        "- CSV location: ~/.codeblocks/anxiety_monitor/sessions/\n"
        "- Popup warnings: Disabled (non-intrusive)\n"
        "- Sound alerts: Disabled\n\n"
        "Settings dialog coming in future version.",
        "Anxiety Monitor Settings",
        wxOK | wxICON_INFORMATION
    );
}

// ============================================================================
// Status & State
// ============================================================================

bool AnxietyMonitorPlugin::IsSessionActive() const
{
    return m_sessionState != SessionState::STOPPED;
}

SessionState AnxietyMonitorPlugin::GetSessionState() const
{
    return m_sessionState;
}

double AnxietyMonitorPlugin::GetCurrentScore() const
{
    if (m_dataCollector) {
        return m_dataCollector->GetAnxietyScore();
    }
    return 0.0;
}

// ============================================================================
// Timer & Auto-Save
// ============================================================================

void AnxietyMonitorPlugin::OnTimerUpdate(wxTimerEvent& event)
{
    (void)event;
    
    if (m_sessionState == SessionState::RUNNING) {
        AutoSaveMetrics();
    }
    
    UpdateStatusBar();
    
    // Update panel if exists
    if (m_panel && m_dataCollector) {
        m_panel->UpdateMetrics(m_dataCollector->GetCurrentSnapshot());
    }
}

void AnxietyMonitorPlugin::AutoSaveMetrics()
{
    if (!m_dataCollector || !m_csvWriter) {
        return;
    }
    
    // Get current metrics snapshot
    MetricsSnapshot snapshot = m_dataCollector->GetCurrentSnapshot();
    
    // Calculate and update anxiety score
    if (m_scorer) {
        snapshot.anxietyScore = m_scorer->CalculateScore(snapshot);
        RiskLevel level = m_scorer->GetRiskLevel(snapshot.anxietyScore);
        snapshot.riskLevel = GetRiskLevelLabel(level);
        
        // Check if we should show a warning (respects cooldown, only HIGH/CRITICAL)
        // But since user wants non-intrusive, this is disabled by default
        if (m_settings.showPopupWarnings && m_scorer->ShouldShowWarning(level)) {
            std::string recommendation = m_scorer->GetRecommendation(level);
            ShowAnxietyNotification(nullptr, level, recommendation);
        }
    }
    
    // Write to CSV (auto-flushes immediately for data safety)
    m_csvWriter->WriteSnapshot(snapshot);
}

void AnxietyMonitorPlugin::ForceSave()
{
    // Force immediate write of current state
    if (m_sessionState != SessionState::STOPPED && m_csvWriter && m_dataCollector) {
        MetricsSnapshot snapshot = m_dataCollector->GetCurrentSnapshot();
        if (m_scorer) {
            snapshot.anxietyScore = m_scorer->CalculateScore(snapshot);
            snapshot.riskLevel = GetRiskLevelLabel(m_scorer->GetRiskLevel(snapshot.anxietyScore));
        }
        m_csvWriter->WriteSnapshot(snapshot);
        m_csvWriter->Flush();
        
        wxLogMessage("AnxietyMonitor: Data force-saved.");
    }
}

// ============================================================================
// UI Updates
// ============================================================================

void AnxietyMonitorPlugin::UpdateStatusBar()
{
    // Update status bar with current metrics
    // In real CB plugin, we'd use Manager::Get()->GetAppFrame()->SetStatusText()
    
    if (!m_dataCollector || !m_statusBarManager) {
        return;
    }
    
    if (m_sessionState == SessionState::STOPPED) {
        // Set idle status
        // SetStatusText("[Anxiety Monitor] Ready - Click Start to begin monitoring");
        return;
    }
    
    MetricsSnapshot snapshot = m_dataCollector->GetCurrentSnapshot();
    std::string statusText = m_statusBarManager->FormatStatusText(snapshot);
    
    // In real plugin:
    // Manager::Get()->GetAppFrame()->SetStatusText(statusText, 1);
    
    // For debug/logging:
    // wxLogMessage("Status: %s", statusText.c_str());
}

void AnxietyMonitorPlugin::UpdateUI()
{
    EnableToolbarButtons(m_sessionState != SessionState::STOPPED);
    
    if (m_panel) {
        m_panel->SetSessionState(m_sessionState);
    }
}

void AnxietyMonitorPlugin::EnableToolbarButtons(bool sessionActive)
{
    if (!m_toolBar) return;
    
    m_toolBar->EnableTool(ID_ANXIETY_START, !sessionActive);
    m_toolBar->EnableTool(ID_ANXIETY_PAUSE, sessionActive);
    m_toolBar->EnableTool(ID_ANXIETY_EXPORT, sessionActive);
}

// ============================================================================
// Button Event Handlers
// ============================================================================

void AnxietyMonitorPlugin::OnStartButton(wxCommandEvent& event)
{
    (void)event;
    StartSession();
}

void AnxietyMonitorPlugin::OnPauseButton(wxCommandEvent& event)
{
    (void)event;
    PauseSession();
}

void AnxietyMonitorPlugin::OnExportButton(wxCommandEvent& event)
{
    (void)event;
    ExportSession();
}

void AnxietyMonitorPlugin::OnSettingsButton(wxCommandEvent& event)
{
    (void)event;
    ShowSettings();
}

// ============================================================================
// Menu Event Handlers
// ============================================================================

void AnxietyMonitorPlugin::OnMenuStart(wxCommandEvent& event)
{
    (void)event;
    StartSession();
}

void AnxietyMonitorPlugin::OnMenuPause(wxCommandEvent& event)
{
    (void)event;
    PauseSession();
}

void AnxietyMonitorPlugin::OnMenuEnd(wxCommandEvent& event)
{
    (void)event;
    EndSession();
}

void AnxietyMonitorPlugin::OnMenuShowPanel(wxCommandEvent& event)
{
    (void)event;
    
    // In real CB plugin, we'd toggle a dockable pane:
    // CodeBlocksDockEvent evt(cbEVT_SHOW_DOCK_WINDOW);
    // evt.pWindow = m_panel;
    // Manager::Get()->ProcessEvent(evt);
    
    wxMessageBox("Panel toggle - would show/hide dockable metrics panel.",
                 "Anxiety Monitor", wxOK);
}
