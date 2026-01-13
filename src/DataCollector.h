#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <wx/timer.h>
#include <wx/event.h>
#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include "MetricsData.h"

namespace AnxietyMonitor {

/**
 * @class DataCollector
 * @brief Collects and aggregates all 12 behavioral metrics for anxiety detection.
 * 
 * Monitors:
 * - Tier 1 (70%): Keystroke latency, typing speed, error freq, pause ratio, error resolution
 * - Tier 2 (25%): Backspace rate, consecutive errors, undo/redo, idle ratio
 * - Tier 3 (5%): Focus switches, compile success, session fragmentation
 */
class DataCollector {
public:
    DataCollector();
    ~DataCollector();
    
    // Session control
    void StartSession();
    void PauseSession();
    void ResumeSession();
    void EndSession();
    void Reset();
    
    // Event handlers (called from plugin event callbacks)
    void OnKeystroke(bool isBackspace = false);
    void OnUndo();
    void OnRedo();
    void OnCompileStart();
    void OnCompileEnd(int errorCount, int warningCount, bool success);
    void OnEditorFocusChange(bool hasFocus);
    void OnTabChange();
    void OnIdleTick();  // Called periodically to track idle time
    
    // Set context information
    void SetActiveProject(const std::string& projectName);
    void SetActiveFile(const std::string& filePath);
    void SetLanguage(const std::string& language);
    void SetWindowFocused(bool focused);
    
    // Get current metrics snapshot
    MetricsSnapshot GetCurrentSnapshot() const;
    
    // Get individual metrics (for status bar display)
    double GetTypingSpeedWpm() const;
    double GetErrorsPerMinute() const;
    double GetPauseRatio() const;
    double GetBackspaceRate() const;
    double GetAnxietyScore() const;
    RiskLevel GetRiskLevel() const;
    
    // Session state
    SessionState GetSessionState() const { return m_sessionState; }
    bool IsRunning() const { return m_sessionState == SessionState::RUNNING; }

private:
    // Internal calculation methods
    void UpdateDerivedMetrics();
    double CalculateLatencyVariance() const;
    double CalculateTypingSpeed() const;
    double CalculatePauseRatio() const;
    double CalculateIdleRatio() const;
    double CalculateSessionFragmentation() const;
    double CalculateCompileSuccessRate() const;
    
    // Get current time in milliseconds
    long GetCurrentTimeMs() const;
    
    // Check for pause (>2s gap) or break (>30s gap)
    void CheckForPauseOrBreak();

private:
    SessionState m_sessionState;
    mutable std::mutex m_mutex;
    
    // Session timing
    std::chrono::steady_clock::time_point m_sessionStart;
    std::chrono::steady_clock::time_point m_lastKeystrokeTime;
    std::chrono::steady_clock::time_point m_lastActivityTime;
    std::chrono::steady_clock::time_point m_lastCompileEndTime;
    
    // Rolling buffers for metrics
    RollingBuffer<long> m_interKeyDelays;       // Keystroke intervals in ms
    RollingBuffer<double> m_typingSpeedSamples; // WPM samples
    
    // Counters
    long m_totalKeystrokes;
    long m_keystrokesInWindow;  // Last 5 minutes
    long m_backspaceCount;
    long m_undoCount;
    long m_redoCount;
    
    // Compile tracking
    int m_compileAttempts;
    int m_successfulCompiles;
    int m_totalErrors;
    int m_errorsInWindow;  // Errors in last 5 minutes
    std::vector<CompileEvent> m_recentCompiles;  // For error resolution time
    
    // Time tracking
    long m_totalPauseTimeMs;     // Time in pauses (>2s gaps)
    long m_totalIdleTimeMs;      // Time idle (no activity)
    long m_totalActiveTimeMs;    // Time actively typing
    int m_breakCount;            // Breaks > 30s
    
    // Focus tracking
    int m_focusSwitchCount;
    bool m_windowHasFocus;
    
    // Context
    std::string m_projectName;
    std::string m_activeFilePath;
    std::string m_language;
    
    // Cached computed values
    double m_cachedLatencyVariance;
    double m_cachedTypingSpeed;
    double m_cachedAnxietyScore;
    RiskLevel m_cachedRiskLevel;
    
    // Settings reference
    PluginSettings m_settings;
};

} // namespace AnxietyMonitor

#endif // DATA_COLLECTOR_H
