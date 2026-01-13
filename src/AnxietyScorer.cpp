#include "AnxietyScorer.h"
#include <algorithm>
#include <cmath>

namespace AnxietyMonitor {

AnxietyScorer::AnxietyScorer()
    : m_maxLatencyVariance(500.0)        // Higher variance = more anxiety
    , m_baselineTypingSpeed(40.0)        // WPM below this increases anxiety
    , m_maxErrorsPerMinute(10.0)         // 10 errors/min = max anxiety
    , m_maxPauseRatio(0.5)               // 50% pause time = max anxiety
    , m_maxErrorResolutionSec(300.0)     // 5 minutes to fix = max anxiety
    , m_maxBackspaceRate(20.0)           // 20% backspaces = max anxiety
    , m_maxConsecutiveErrors(10)         // 10 consecutive errors = max
    , m_maxUndoRedoCount(30)             // 30 undo/redo per 10 min = max
    , m_maxIdleRatio(0.6)                // 60% idle = max anxiety indicator
    , m_maxFocusSwitchesPerMin(5.0)      // 5 switches/min = max
    , m_maxFragmentation(0.3)            // 30% breaks = max fragmentation
    , m_warningCooldownMinutes(10)
{
    m_lastWarningTime = std::chrono::steady_clock::now() - std::chrono::hours(1);
}

double AnxietyScorer::Clamp01(double value) const
{
    return std::max(0.0, std::min(1.0, value));
}

double AnxietyScorer::NormalizeLatencyVariance(double latencyMs) const
{
    // Higher variance = higher anxiety
    return Clamp01(latencyMs / m_maxLatencyVariance);
}

double AnxietyScorer::NormalizeTypingSpeed(double wpm) const
{
    // Lower speed = higher anxiety (inverted)
    // If speed is 0, could be idle - treat as moderate concern
    if (wpm <= 0) return 0.3;
    
    // Calculate deviation from baseline
    double deviation = (m_baselineTypingSpeed - wpm) / m_baselineTypingSpeed;
    return Clamp01(deviation);
}

double AnxietyScorer::NormalizeErrorFrequency(double errorsPerMin) const
{
    return Clamp01(errorsPerMin / m_maxErrorsPerMinute);
}

double AnxietyScorer::NormalizePauseRatio(double ratio) const
{
    return Clamp01(ratio / m_maxPauseRatio);
}

double AnxietyScorer::NormalizeErrorResolution(double seconds) const
{
    return Clamp01(seconds / m_maxErrorResolutionSec);
}

double AnxietyScorer::NormalizeBackspaceRate(double rate) const
{
    return Clamp01(rate / m_maxBackspaceRate);
}

double AnxietyScorer::NormalizeConsecutiveErrors(int count) const
{
    return Clamp01(static_cast<double>(count) / m_maxConsecutiveErrors);
}

double AnxietyScorer::NormalizeUndoRedo(int count) const
{
    return Clamp01(static_cast<double>(count) / m_maxUndoRedoCount);
}

double AnxietyScorer::NormalizeIdleRatio(double ratio) const
{
    return Clamp01(ratio / m_maxIdleRatio);
}

double AnxietyScorer::NormalizeFocusSwitches(double perMin) const
{
    return Clamp01(perMin / m_maxFocusSwitchesPerMin);
}

double AnxietyScorer::NormalizeCompileSuccess(double percent) const
{
    // Lower success rate = higher anxiety (inverted)
    return Clamp01((100.0 - percent) / 100.0);
}

double AnxietyScorer::NormalizeFragmentation(double ratio) const
{
    return Clamp01(ratio / m_maxFragmentation);
}

double AnxietyScorer::CalculateScore(const MetricsSnapshot& metrics) const
{
    // Normalize all metrics to 0-1 scale
    double latencyNorm = NormalizeLatencyVariance(metrics.latencyVarianceMs);
    double speedNorm = NormalizeTypingSpeed(metrics.typingSpeedWpm);
    double errorFreqNorm = NormalizeErrorFrequency(metrics.errorFreqPerMin);
    double pauseNorm = NormalizePauseRatio(metrics.pauseRatio);
    double resTimeNorm = NormalizeErrorResolution(metrics.errorResolutionTime);
    
    double backspaceNorm = NormalizeBackspaceRate(metrics.backspaceRate);
    double consecErrNorm = NormalizeConsecutiveErrors(metrics.consecutiveErrors);
    double undoNorm = NormalizeUndoRedo(metrics.undoRedoCount);
    double idleNorm = NormalizeIdleRatio(metrics.idleRatio);
    
    double focusNorm = NormalizeFocusSwitches(metrics.focusSwitches);
    double successNorm = NormalizeCompileSuccess(metrics.compileSuccessRate);
    double fragNorm = NormalizeFragmentation(metrics.sessionFragmentation);
    
    // Calculate tier scores (each tier is normalized 0-1)
    double tier1 = T1_LATENCY_WEIGHT * latencyNorm +
                   T1_SPEED_WEIGHT * speedNorm +
                   T1_ERROR_FREQ_WEIGHT * errorFreqNorm +
                   T1_PAUSE_WEIGHT * pauseNorm +
                   T1_RESOLUTION_WEIGHT * resTimeNorm;
    
    double tier2 = T2_BACKSPACE_WEIGHT * backspaceNorm +
                   T2_CONSEC_ERR_WEIGHT * consecErrNorm +
                   T2_UNDO_WEIGHT * undoNorm +
                   T2_IDLE_WEIGHT * idleNorm;
    
    double tier3 = T3_FOCUS_WEIGHT * focusNorm +
                   T3_SUCCESS_WEIGHT * successNorm +
                   T3_FRAG_WEIGHT * fragNorm;
    
    // Ensure tier scores are normalized (they should be if sub-weights sum to 1)
    tier1 = Clamp01(tier1);
    tier2 = Clamp01(tier2);
    tier3 = Clamp01(tier3);
    
    // Final weighted score (0-100 scale)
    double score = (TIER1_WEIGHT * tier1 + 
                    TIER2_WEIGHT * tier2 + 
                    TIER3_WEIGHT * tier3) * 100.0;
    
    return std::max(0.0, std::min(100.0, score));
}

RiskLevel AnxietyScorer::GetRiskLevel(double score) const
{
    if (score <= 30.0) {
        return RiskLevel::LOW;
    } else if (score <= 60.0) {
        return RiskLevel::MODERATE;
    } else if (score <= 80.0) {
        return RiskLevel::HIGH;
    } else {
        return RiskLevel::CRITICAL;
    }
}

std::string AnxietyScorer::GetRecommendation(RiskLevel level) const
{
    switch (level) {
        case RiskLevel::LOW:
            return "You're doing great! Maintain your coding flow.";
        
        case RiskLevel::MODERATE:
            return "Consider taking a short break. Stretch and relax your eyes.";
        
        case RiskLevel::HIGH:
            return "Elevated stress detected. Try:\n"
                   "- Take a 5-minute walk\n"
                   "- Practice deep breathing\n"
                   "- Ask a peer for help";
        
        case RiskLevel::CRITICAL:
            return "High stress levels detected. Please:\n"
                   "- Step away from the computer\n"
                   "- Take a 10-minute break\n"
                   "- Consider talking to someone\n"
                   "- The problem will be easier after a break";
        
        default:
            return "";
    }
}

bool AnxietyScorer::ShouldShowWarning(RiskLevel level) const
{
    // Only show warnings for HIGH and CRITICAL levels
    if (level < RiskLevel::HIGH) {
        return false;
    }
    
    // Check cooldown period
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
        now - m_lastWarningTime).count();
    
    if (elapsed >= m_warningCooldownMinutes) {
        m_lastWarningTime = now;
        return true;
    }
    
    return false;
}

void AnxietyScorer::ResetWarningCooldown()
{
    m_lastWarningTime = std::chrono::steady_clock::now() - std::chrono::hours(1);
}

} // namespace AnxietyMonitor
