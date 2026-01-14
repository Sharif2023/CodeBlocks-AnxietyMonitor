#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#ifdef STANDALONE_BUILD
#include "wx_stubs.h"
#else
#include <wx/gauge.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/wx.h>

#endif

#include "MetricsData.h"
#include <string>
#include <vector>


namespace AnxietyMonitor {

// Forward declarations
class DataCollector;

/**
 * @class AnxietyPanel
 * @brief Dockable panel showing real-time anxiety metrics and trends.
 *
 * Features:
 * - Live anxiety score with color indicator
 * - 5-minute trend sparkline
 * - Current metrics display
 * - Recent events log
 * - Recommendations (non-intrusive, only for HIGH/CRITICAL)
 */
class AnxietyPanel : public wxPanel {
public:
  AnxietyPanel(wxWindow *parent, wxWindowID id = wxID_ANY);
  virtual ~AnxietyPanel();

  /**
   * @brief Update display with new metrics data.
   */
  void UpdateMetrics(const MetricsSnapshot &snapshot);

  /**
   * @brief Add an event to the recent events log.
   */
  void AddEvent(const std::string &event);

  /**
   * @brief Clear the events log.
   */
  void ClearEvents();

  /**
   * @brief Set session state (affects display).
   */
  void SetSessionState(SessionState state);

  /**
   * @brief Update the trend data (5-minute rolling).
   */
  void UpdateTrend(double score);

private:
  void CreateControls();
  void UpdateScoreDisplay(double score, RiskLevel level);
  void UpdateMetricsDisplay(const MetricsSnapshot &snapshot);
  void UpdateTrendDisplay();
  wxColour GetRiskColor(RiskLevel level) const;
  std::string GetTimeString() const;

private:
  // Main score display
  wxStaticText *m_scoreLabel;
  wxStaticText *m_scoreValue;
  wxStaticText *m_riskLabel;
  wxGauge *m_scoreGauge;

  // Metrics display
  wxStaticText *m_errorsLabel;
  wxStaticText *m_typingLabel;
  wxStaticText *m_pauseLabel;
  wxStaticText *m_backspaceLabel;

  // Trend display (simplified as numbers)
  wxStaticText *m_trendLabel;
  std::vector<double> m_trendData;
  static const size_t MAX_TREND_POINTS = 10;

  // Events log
  wxListBox *m_eventsList;
  static const int MAX_EVENTS = 15;

  // Recommendations
  wxStaticText *m_recommendationLabel;

  // Session state
  SessionState m_sessionState;
};

/**
 * @class StatusBarManager
 * @brief Manages the status bar display for anxiety metrics.
 *
 * Format: [Anxiety Monitor] Score: 24 [OK] | Errors: 2/min | Typing: 45wpm
 */
class StatusBarManager {
public:
  StatusBarManager();
  ~StatusBarManager() = default;

  /**
   * @brief Format the status bar text from metrics.
   */
  std::string FormatStatusText(const MetricsSnapshot &snapshot) const;

  /**
   * @brief Format short status for compact display.
   */
  std::string FormatShortStatus(double score, RiskLevel level) const;

private:
  std::string FormatSpeedChange(double currentWpm, double baselineWpm) const;
};

/**
 * @brief Simple message box for non-intrusive notifications.
 * Only shown for HIGH/CRITICAL and respects cooldown.
 */
void ShowAnxietyNotification(wxWindow *parent, RiskLevel level,
                             const std::string &message);

} // namespace AnxietyMonitor

#endif // UI_COMPONENTS_H
