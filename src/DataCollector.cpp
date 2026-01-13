#include "DataCollector.h"
#include "AnxietyScorer.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace AnxietyMonitor {

DataCollector::DataCollector()
    : m_sessionState(SessionState::STOPPED)
    , m_totalKeystrokes(0)
    , m_keystrokesInWindow(0)
    , m_backspaceCount(0)
    , m_undoCount(0)
    , m_redoCount(0)
    , m_compileAttempts(0)
    , m_successfulCompiles(0)
    , m_totalErrors(0)
    , m_errorsInWindow(0)
    , m_totalPauseTimeMs(0)
    , m_totalIdleTimeMs(0)
    , m_totalActiveTimeMs(0)
    , m_breakCount(0)
    , m_focusSwitchCount(0)
    , m_windowHasFocus(true)
    , m_cachedLatencyVariance(0.0)
    , m_cachedTypingSpeed(0.0)
    , m_cachedAnxietyScore(0.0)
    , m_cachedRiskLevel(RiskLevel::LOW)
    , m_interKeyDelays(300)   // ~5 minutes worth at typical typing
    , m_typingSpeedSamples(10)
{
}

DataCollector::~DataCollector()
{
    EndSession();
}

void DataCollector::StartSession()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_sessionState != SessionState::STOPPED) {
        return;
    }
    
    Reset();
    m_sessionState = SessionState::RUNNING;
    m_sessionStart = std::chrono::steady_clock::now();
    m_lastKeystrokeTime = m_sessionStart;
    m_lastActivityTime = m_sessionStart;
}

void DataCollector::PauseSession()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_sessionState == SessionState::RUNNING) {
        m_sessionState = SessionState::PAUSED;
    }
}

void DataCollector::ResumeSession()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_sessionState == SessionState::PAUSED) {
        m_sessionState = SessionState::RUNNING;
        m_lastActivityTime = std::chrono::steady_clock::now();
    }
}

void DataCollector::EndSession()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessionState = SessionState::STOPPED;
}

void DataCollector::Reset()
{
    m_totalKeystrokes = 0;
    m_keystrokesInWindow = 0;
    m_backspaceCount = 0;
    m_undoCount = 0;
    m_redoCount = 0;
    m_compileAttempts = 0;
    m_successfulCompiles = 0;
    m_totalErrors = 0;
    m_errorsInWindow = 0;
    m_totalPauseTimeMs = 0;
    m_totalIdleTimeMs = 0;
    m_totalActiveTimeMs = 0;
    m_breakCount = 0;
    m_focusSwitchCount = 0;
    
    m_interKeyDelays.clear();
    m_typingSpeedSamples.clear();
    m_recentCompiles.clear();
    
    m_cachedLatencyVariance = 0.0;
    m_cachedTypingSpeed = 0.0;
    m_cachedAnxietyScore = 0.0;
    m_cachedRiskLevel = RiskLevel::LOW;
}

long DataCollector::GetCurrentTimeMs() const
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

void DataCollector::OnKeystroke(bool isBackspace)
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto now = std::chrono::steady_clock::now();
    
    // Calculate inter-key delay
    auto delayMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastKeystrokeTime).count();
    
    // Check for pause or break before recording
    if (delayMs > m_settings.pauseThresholdMs) {
        m_totalPauseTimeMs += delayMs;
        
        if (delayMs > m_settings.breakThresholdMs) {
            ++m_breakCount;
        }
    } else {
        // Normal keystroke - record inter-key delay
        m_interKeyDelays.push(delayMs);
        m_totalActiveTimeMs += delayMs;
    }
    
    // Update counters
    ++m_totalKeystrokes;
    ++m_keystrokesInWindow;
    
    if (isBackspace) {
        ++m_backspaceCount;
    }
    
    m_lastKeystrokeTime = now;
    m_lastActivityTime = now;
    
    // Update derived metrics periodically
    if (m_totalKeystrokes % 10 == 0) {
        UpdateDerivedMetrics();
    }
}

void DataCollector::OnUndo()
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_undoCount;
    m_lastActivityTime = std::chrono::steady_clock::now();
}

void DataCollector::OnRedo()
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_redoCount;
    m_lastActivityTime = std::chrono::steady_clock::now();
}

void DataCollector::OnCompileStart()
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    CompileEvent event;
    event.startTime = std::chrono::steady_clock::now();
    m_recentCompiles.push_back(event);
    
    m_lastActivityTime = event.startTime;
}

void DataCollector::OnCompileEnd(int errorCount, int warningCount, bool success)
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto now = std::chrono::steady_clock::now();
    
    ++m_compileAttempts;
    
    if (success) {
        ++m_successfulCompiles;
    }
    
    m_totalErrors += errorCount;
    m_errorsInWindow += errorCount;
    
    // Update the most recent compile event
    if (!m_recentCompiles.empty()) {
        auto& lastCompile = m_recentCompiles.back();
        lastCompile.endTime = now;
        lastCompile.errorCount = errorCount;
        lastCompile.warningCount = warningCount;
        lastCompile.success = success;
    }
    
    m_lastCompileEndTime = now;
    m_lastActivityTime = now;
    
    // Keep only last 20 compiles
    if (m_recentCompiles.size() > 20) {
        m_recentCompiles.erase(m_recentCompiles.begin());
    }
    
    UpdateDerivedMetrics();
}

void DataCollector::OnEditorFocusChange(bool hasFocus)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (hasFocus != m_windowHasFocus) {
        m_windowHasFocus = hasFocus;
        
        if (m_sessionState == SessionState::RUNNING) {
            // Could track focus loss time here for idle calculation
        }
    }
}

void DataCollector::OnTabChange()
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_focusSwitchCount;
    m_lastActivityTime = std::chrono::steady_clock::now();
}

void DataCollector::OnIdleTick()
{
    if (m_sessionState != SessionState::RUNNING) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto now = std::chrono::steady_clock::now();
    auto idleMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastActivityTime).count();
    
    // Track idle time if no activity for more than 2 seconds
    if (idleMs > m_settings.pauseThresholdMs) {
        m_totalIdleTimeMs += 1000;  // Add tick interval (assumed 1 second)
    }
}

void DataCollector::SetActiveProject(const std::string& projectName)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_projectName = projectName;
}

void DataCollector::SetActiveFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activeFilePath = filePath;
    
    // Attempt to detect language from file extension
    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos) {
        std::string ext = filePath.substr(dotPos + 1);
        if (ext == "cpp" || ext == "cxx" || ext == "cc") {
            m_language = "C++";
        } else if (ext == "c") {
            m_language = "C";
        } else if (ext == "h" || ext == "hpp") {
            m_language = "Header";
        } else if (ext == "py") {
            m_language = "Python";
        } else if (ext == "java") {
            m_language = "Java";
        } else {
            m_language = ext;
        }
    }
}

void DataCollector::SetLanguage(const std::string& language)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_language = language;
}

void DataCollector::SetWindowFocused(bool focused)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_windowHasFocus = focused;
}

void DataCollector::UpdateDerivedMetrics()
{
    m_cachedLatencyVariance = CalculateLatencyVariance();
    m_cachedTypingSpeed = CalculateTypingSpeed();
    
    // Calculate anxiety score using AnxietyScorer
    MetricsSnapshot snapshot = GetCurrentSnapshot();
    AnxietyScorer scorer;
    m_cachedAnxietyScore = scorer.CalculateScore(snapshot);
    m_cachedRiskLevel = scorer.GetRiskLevel(m_cachedAnxietyScore);
}

double DataCollector::CalculateLatencyVariance() const
{
    return m_interKeyDelays.stddev();
}

double DataCollector::CalculateTypingSpeed() const
{
    if (m_totalActiveTimeMs <= 0) return 0.0;
    
    // Characters per minute (assume 5 characters per word for WPM)
    double activeMinutes = m_totalActiveTimeMs / 60000.0;
    if (activeMinutes < 0.1) return 0.0;
    
    double charsPerMinute = m_keystrokesInWindow / activeMinutes;
    return charsPerMinute / 5.0;  // Convert to WPM
}

double DataCollector::CalculatePauseRatio() const
{
    long totalTime = m_totalActiveTimeMs + m_totalPauseTimeMs;
    if (totalTime <= 0) return 0.0;
    
    return static_cast<double>(m_totalPauseTimeMs) / totalTime;
}

double DataCollector::CalculateIdleRatio() const
{
    auto now = std::chrono::steady_clock::now();
    auto sessionMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_sessionStart).count();
    
    if (sessionMs <= 0) return 0.0;
    
    return static_cast<double>(m_totalIdleTimeMs) / sessionMs;
}

double DataCollector::CalculateSessionFragmentation() const
{
    auto now = std::chrono::steady_clock::now();
    auto sessionMinutes = std::chrono::duration_cast<std::chrono::minutes>(
        now - m_sessionStart).count();
    
    if (sessionMinutes <= 0) return 0.0;
    
    return static_cast<double>(m_breakCount) / sessionMinutes;
}

double DataCollector::CalculateCompileSuccessRate() const
{
    if (m_compileAttempts <= 0) return 100.0;  // No compiles = 100% success
    
    return (static_cast<double>(m_successfulCompiles) / m_compileAttempts) * 100.0;
}

MetricsSnapshot DataCollector::GetCurrentSnapshot() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    MetricsSnapshot snapshot;
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
    snapshot.timestamp = oss.str();
    
    // Session info
    oss.str("");
    oss << std::put_time(tm, "%Y%m%d%H%M%S");
    snapshot.sessionId = "session_" + oss.str();
    snapshot.projectName = m_projectName;
    snapshot.filePath = m_activeFilePath;
    snapshot.language = m_language;
    
    // Calculate session duration
    auto sessionDuration = std::chrono::steady_clock::now() - m_sessionStart;
    double sessionMinutes = std::chrono::duration_cast<std::chrono::seconds>(sessionDuration).count() / 60.0;
    if (sessionMinutes < 0.1) sessionMinutes = 0.1;  // Avoid division by zero
    
    // Tier 1 Metrics
    snapshot.typingSpeedWpm = m_cachedTypingSpeed;
    snapshot.latencyVarianceMs = m_cachedLatencyVariance;
    snapshot.errorFreqPerMin = m_errorsInWindow / sessionMinutes;
    snapshot.pauseRatio = CalculatePauseRatio();
    
    // Error resolution time (average time between error and next successful compile)
    double totalResolutionTime = 0.0;
    int resolutionCount = 0;
    for (size_t i = 1; i < m_recentCompiles.size(); ++i) {
        if (!m_recentCompiles[i-1].success && m_recentCompiles[i].success) {
            auto resTime = std::chrono::duration_cast<std::chrono::seconds>(
                m_recentCompiles[i].endTime - m_recentCompiles[i-1].endTime).count();
            totalResolutionTime += resTime;
            ++resolutionCount;
        }
    }
    snapshot.errorResolutionTime = resolutionCount > 0 ? 
        totalResolutionTime / resolutionCount : 0.0;
    
    // Tier 2 Metrics
    snapshot.backspaceRate = m_totalKeystrokes > 0 ? 
        (static_cast<double>(m_backspaceCount) / m_totalKeystrokes) * 100.0 : 0.0;
    snapshot.consecutiveErrors = m_errorsInWindow;
    snapshot.undoRedoCount = static_cast<int>(m_undoCount + m_redoCount);
    snapshot.idleRatio = CalculateIdleRatio();
    
    // Tier 3 Metrics
    snapshot.focusSwitches = m_focusSwitchCount / sessionMinutes;
    snapshot.compileSuccessRate = CalculateCompileSuccessRate();
    snapshot.sessionFragmentation = CalculateSessionFragmentation();
    
    // Computed values
    snapshot.anxietyScore = m_cachedAnxietyScore;
    snapshot.riskLevel = GetRiskLevelLabel(m_cachedRiskLevel);
    
    // Metadata
    snapshot.timestampBatch = snapshot.timestamp;
    snapshot.cpuUsage = 0.0;   // Would need platform-specific code
    snapshot.memoryUsage = 0.0; // Would need platform-specific code
    snapshot.windowFocused = m_windowHasFocus;
    snapshot.keystrokesTotal = m_totalKeystrokes;
    snapshot.compileAttempts = m_compileAttempts;
    snapshot.errorCountTotal = m_totalErrors;
    
    return snapshot;
}

double DataCollector::GetTypingSpeedWpm() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cachedTypingSpeed;
}

double DataCollector::GetErrorsPerMinute() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto sessionDuration = std::chrono::steady_clock::now() - m_sessionStart;
    double sessionMinutes = std::chrono::duration_cast<std::chrono::seconds>(sessionDuration).count() / 60.0;
    if (sessionMinutes < 0.1) return 0.0;
    
    return m_errorsInWindow / sessionMinutes;
}

double DataCollector::GetPauseRatio() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return CalculatePauseRatio();
}

double DataCollector::GetBackspaceRate() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_totalKeystrokes <= 0) return 0.0;
    return (static_cast<double>(m_backspaceCount) / m_totalKeystrokes) * 100.0;
}

double DataCollector::GetAnxietyScore() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cachedAnxietyScore;
}

RiskLevel DataCollector::GetRiskLevel() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cachedRiskLevel;
}

} // namespace AnxietyMonitor
