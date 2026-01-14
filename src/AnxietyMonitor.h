#ifndef ANXIETY_MONITOR_H
#define ANXIETY_MONITOR_H

// wxWidgets includes (or stubs for standalone build)
#ifdef STANDALONE_BUILD
#include "wx_stubs.h"
#else
#include <wx/event.h>
#include <wx/timer.h>
#include <wx/wx.h>

#endif

// Standard includes
#include <memory>
#include <string>

// Plugin includes
#include "MetricsData.h"

// Forward declarations
namespace AnxietyMonitor {
class DataCollector;
class CSVWriter;
class AnxietyScorer;
class AnxietyPanel;
class EventHandlers;
class StatusBarManager;
} // namespace AnxietyMonitor

/**
 * @class AnxietyMonitorPlugin
 * @brief Main Code::Blocks plugin class for programming anxiety detection.
 *
 * Features:
 * - Real-time monitoring of 12 behavioral metrics
 * - Live anxiety scoring (0-100) with risk levels
 * - Auto-save CSV data every 30 seconds
 * - Auto-save on session stop or plugin exit
 * - Non-intrusive UI (status bar + optional dockable panel)
 * - User-friendly with minimal interruptions
 *
 * Research basis:
 * - Lau (2018): Keystroke dynamics
 * - Yu et al. (2025): Programming anxiety ML
 * - Becker (2016): Compiler error metrics
 * - Perera (2023): Real-time stress detection
 */

// Note: In real Code::Blocks plugin, this inherits from cbPlugin
// For standalone compilation, we provide a simplified base
#ifdef CODEBLOCKS_SDK_INCLUDED
#include <cbplugin.h>
#include <sdk.h>
class AnxietyMonitorPlugin : public cbPlugin
#else
// Simplified plugin base for compilation without CB SDK
class cbConfigurationPanel {
public:
    virtual ~cbConfigurationPanel() {}
};

class cbPlugin : public wxEvtHandler {
public:
  enum PluginType { ptTool = 1 };
  virtual void OnAttach() {}
  virtual void OnRelease(bool appShutDown) { (void)appShutDown; }
  virtual int GetConfigurationGroup() const { return 0; }
  virtual bool BuildToolBar(wxToolBar *toolBar) {
    (void)toolBar;
    return false;
  }
  virtual void BuildMenu(wxMenuBar *menuBar) { (void)menuBar; }
  virtual void BuildModuleMenu(const int type, wxMenu *menu) {
    (void)type;
    (void)menu;
  }
  virtual cbConfigurationPanel *GetConfigurationPanel(wxWindow *parent) {
    (void)parent;
    return nullptr;
  }
};

class AnxietyMonitorPlugin : public cbPlugin
#endif
{
public:
  AnxietyMonitorPlugin();
  virtual ~AnxietyMonitorPlugin();

  // =========================================================================
  // Plugin Lifecycle (cbPlugin interface)
  // =========================================================================

  /**
   * @brief Called when the plugin is attached to Code::Blocks.
   * Initialize UI, event handlers, and prepare for monitoring.
   */
  void OnAttach() override;

  /**
   * @brief Called when the plugin is released.
   * Auto-save any pending data and clean up resources.
   * @param appShutDown true if Code::Blocks is shutting down
   */
  void OnRelease(bool appShutDown) override;

  /**
   * @brief Get configuration group for settings dialog.
   */
  int GetConfigurationGroup() const override;

  /**
   * @brief Build the plugin's toolbar.
   * Creates Start/Pause/Export/Settings buttons.
   */
  bool BuildToolBar(wxToolBar *toolBar) override;

  /**
   * @brief Add items to the main menu bar.
   */
  void BuildMenu(wxMenuBar *menuBar) override;

  /**
   * @brief Add items to context menus.
   */
  void BuildModuleMenu(const int type, wxMenu *menu) override;

  // =========================================================================
  // Session Control
  // =========================================================================

  /**
   * @brief Start a new monitoring session.
   * Creates new CSV file and begins data collection.
   */
  void StartSession();

  /**
   * @brief Pause the current session.
   * Flushes CSV data but keeps session active.
   */
  void PauseSession();

  /**
   * @brief Resume a paused session.
   */
  void ResumeSession();

  /**
   * @brief End the current session.
   * Auto-saves all data and closes CSV file.
   */
  void EndSession();

  /**
   * @brief Export current session data.
   * Shows file dialog and copies CSV to chosen location.
   */
  void ExportSession();

  /**
   * @brief Show settings dialog.
   */
  void ShowSettings();

  // =========================================================================
  // Status & State
  // =========================================================================

  /**
   * @brief Check if a session is currently active.
   */
  bool IsSessionActive() const;

  /**
   * @brief Get current session state.
   */
  AnxietyMonitor::SessionState GetSessionState() const;

  /**
   * @brief Get current anxiety score.
   */
  double GetCurrentScore() const;

protected:
  // =========================================================================
  // Event Handlers
  // =========================================================================

  void OnTimerUpdate(wxTimerEvent &event);
  void OnStartButton(wxCommandEvent &event);
  void OnPauseButton(wxCommandEvent &event);
  void OnExportButton(wxCommandEvent &event);
  void OnSettingsButton(wxCommandEvent &event);
  void OnMenuStart(wxCommandEvent &event);
  void OnMenuPause(wxCommandEvent &event);
  void OnMenuEnd(wxCommandEvent &event);
  void OnMenuShowPanel(wxCommandEvent &event);

private:
  // =========================================================================
  // Initialization
  // =========================================================================

  void InitializeComponents();
  void RegisterEventHandlers();
  void UnregisterEventHandlers();
  void SetupStatusBar();
  void UpdateStatusBar();
  void UpdateUI();

  // =========================================================================
  // Auto-Save Functions
  // =========================================================================

  /**
   * @brief Write current metrics to CSV file.
   * Called automatically every 30 seconds.
   */
  void AutoSaveMetrics();

  /**
   * @brief Force save all pending data immediately.
   * Called on pause, stop, or exit.
   */
  void ForceSave();

  // =========================================================================
  // UI State Management
  // =========================================================================

  void EnableToolbarButtons(bool sessionActive);

private:
  // Core components
  std::unique_ptr<AnxietyMonitor::DataCollector> m_dataCollector;
  std::unique_ptr<AnxietyMonitor::CSVWriter> m_csvWriter;
  std::unique_ptr<AnxietyMonitor::AnxietyScorer> m_scorer;
  std::unique_ptr<AnxietyMonitor::EventHandlers> m_eventHandlers;
  std::unique_ptr<AnxietyMonitor::StatusBarManager> m_statusBarManager;

  // UI components
  AnxietyMonitor::AnxietyPanel *m_panel; // Owned by wxWidgets
  wxToolBar *m_toolBar;

  // Toolbar button IDs
  int m_startButtonId;
  int m_pauseButtonId;
  int m_exportButtonId;
  int m_settingsButtonId;

  // Timer for periodic updates (30 seconds)
  wxTimer m_updateTimer;
  static const int UPDATE_INTERVAL_MS = 30000; // 30 seconds

  // Session state
  AnxietyMonitor::SessionState m_sessionState;
  std::string m_currentSessionId;

  // Settings
  AnxietyMonitor::PluginSettings m_settings;

  // Flag to track initialization
  bool m_isInitialized;

  // Event table declaration
  wxDECLARE_EVENT_TABLE();
};

// Plugin registration macros (for Code::Blocks)
#ifdef CODEBLOCKS_SDK_INCLUDED
CB_DECLARE_PLUGIN();
#endif

#endif // ANXIETY_MONITOR_H
