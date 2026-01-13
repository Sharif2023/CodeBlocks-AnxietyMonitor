#ifndef METRICS_DATA_H
#define METRICS_DATA_H

#include <string>
#include <vector>
#include <chrono>
#include <deque>
#include <cmath>

namespace AnxietyMonitor {

// ============================================================================
// Risk Level Enumeration
// ============================================================================
enum class RiskLevel {
    LOW = 0,        // 0-30: Green - Normal coding flow
    MODERATE = 1,   // 31-60: Yellow - Elevated stress, suggest break
    HIGH = 2,       // 61-80: Orange - Significant anxiety, intervention needed
    CRITICAL = 3    // 81-100: Red - Acute distress, immediate support
};

// Helper function to get risk level label
inline const char* GetRiskLevelLabel(RiskLevel level) {
    switch (level) {
        case RiskLevel::LOW:      return "LOW";
        case RiskLevel::MODERATE: return "MODERATE";
        case RiskLevel::HIGH:     return "HIGH";
        case RiskLevel::CRITICAL: return "CRITICAL";
        default:                  return "UNKNOWN";
    }
}

// Helper function to get risk level emoji
inline const char* GetRiskLevelEmoji(RiskLevel level) {
    switch (level) {
        case RiskLevel::LOW:      return "[OK]";      // Green circle
        case RiskLevel::MODERATE: return "[!]";       // Yellow circle
        case RiskLevel::HIGH:     return "[!!]";      // Orange circle
        case RiskLevel::CRITICAL: return "[!!!]";     // Red circle
        default:                  return "[?]";
    }
}

// ============================================================================
// Keystroke Event Data
// ============================================================================
struct KeystrokeEvent {
    std::chrono::steady_clock::time_point timestamp;
    bool isBackspace;
    bool isUndo;
    bool isRedo;
    long interKeyDelay;  // milliseconds since last keystroke
};

// ============================================================================
// Compile Event Data
// ============================================================================
struct CompileEvent {
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    int errorCount;
    int warningCount;
    bool success;
};

// ============================================================================
// Session Metrics Snapshot (24 columns for CSV)
// ============================================================================
struct MetricsSnapshot {
    // Identifiers
    std::string timestamp;          // ISO 8601 format
    std::string sessionId;          // Unique session identifier
    std::string projectName;        // Current project name
    std::string filePath;           // Active file path
    std::string language;           // Programming language
    
    // Tier 1 Metrics (High-Predictive - 70% weight)
    double typingSpeedWpm;          // Words per minute
    double latencyVarianceMs;       // Keystroke timing variance
    double errorFreqPerMin;         // Compiler errors per minute
    double pauseRatio;              // Ratio of pause time (>2s gaps)
    double errorResolutionTime;     // Avg seconds to fix errors
    
    // Tier 2 Metrics (Medium-Predictive - 25% weight)
    double backspaceRate;           // Backspaces per 100 keystrokes
    int consecutiveErrors;          // Error count in last 5 minutes
    int undoRedoCount;              // Undo/redo operations per 10 min
    double idleRatio;               // Percentage of idle time
    
    // Tier 3 Metrics (Low-Predictive - 5% weight)
    double focusSwitches;           // Tab changes per minute
    double compileSuccessRate;      // Percentage of successful compiles
    double sessionFragmentation;    // Breaks >30s ratio
    
    // Computed Values
    double anxietyScore;            // 0-100 scale
    std::string riskLevel;          // LOW/MODERATE/HIGH/CRITICAL
    
    // Additional Metadata
    std::string timestampBatch;     // Batch timestamp for grouping
    double cpuUsage;                // System CPU usage
    double memoryUsage;             // System memory usage
    bool windowFocused;             // Is Code::Blocks focused
    long keystrokesTotal;           // Total keystrokes in session
    int compileAttempts;            // Total compile attempts
    int errorCountTotal;            // Total errors in session
};

// ============================================================================
// Rolling Window Buffer for Metrics
// ============================================================================
template<typename T>
class RollingBuffer {
public:
    explicit RollingBuffer(size_t maxSize = 300) : m_maxSize(maxSize) {}
    
    void push(const T& value) {
        if (m_data.size() >= m_maxSize) {
            m_data.pop_front();
        }
        m_data.push_back(value);
    }
    
    void clear() { m_data.clear(); }
    
    size_t size() const { return m_data.size(); }
    
    bool empty() const { return m_data.empty(); }
    
    const std::deque<T>& data() const { return m_data; }
    
    // Calculate mean
    double mean() const {
        if (m_data.empty()) return 0.0;
        double sum = 0.0;
        for (const auto& v : m_data) {
            sum += static_cast<double>(v);
        }
        return sum / m_data.size();
    }
    
    // Calculate standard deviation
    double stddev() const {
        if (m_data.size() < 2) return 0.0;
        double avg = mean();
        double sumSq = 0.0;
        for (const auto& v : m_data) {
            double diff = static_cast<double>(v) - avg;
            sumSq += diff * diff;
        }
        return std::sqrt(sumSq / (m_data.size() - 1));
    }

private:
    std::deque<T> m_data;
    size_t m_maxSize;
};

// ============================================================================
// Session State
// ============================================================================
enum class SessionState {
    STOPPED,
    RUNNING,
    PAUSED
};

// ============================================================================
// Configuration Settings (User-friendly defaults)
// ============================================================================
struct PluginSettings {
    // Update intervals
    int statusUpdateIntervalMs = 30000;     // 30 seconds
    int csvWriteIntervalMs = 30000;         // 30 seconds
    
    // Rolling window duration (5 minutes)
    int rollingWindowSeconds = 300;
    
    // Thresholds for metrics normalization
    double maxLatencyVariance = 500.0;      // ms
    double baselineTypingSpeed = 40.0;      // WPM
    double maxErrorsPerMinute = 10.0;
    double maxPauseRatio = 0.5;             // 50%
    double maxBackspaceRate = 20.0;         // per 100 keystrokes
    
    // Pause detection
    int pauseThresholdMs = 2000;            // 2 seconds
    int breakThresholdMs = 30000;           // 30 seconds (fragmentation)
    
    // UI preferences (non-intrusive by default)
    bool showPopupWarnings = false;         // Disabled by default - not intrusive
    bool playSoundAlerts = false;           // Disabled by default
    bool showTrayNotifications = false;     // Disabled by default
    int warningCooldownMinutes = 10;        // Minimum time between warnings
    
    // CSV settings
    std::string csvOutputDir;               // Will be set to ~/.codeblocks/anxiety_monitor/sessions/
    bool autoSaveOnExit = true;             // Auto-save when exiting
    bool autoSaveOnStop = true;             // Auto-save when stopping session
};

} // namespace AnxietyMonitor

#endif // METRICS_DATA_H
