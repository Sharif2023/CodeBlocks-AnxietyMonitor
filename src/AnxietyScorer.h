#ifndef ANXIETY_SCORER_H
#define ANXIETY_SCORER_H

#include "MetricsData.h"

namespace AnxietyMonitor {

/**
 * @class AnxietyScorer
 * @brief Calculates anxiety scores using research-validated weighted formulas.
 * 
 * Based on:
 * - Yu et al. (2025) - Programming anxiety ML weights
 * - Lau (2018) - Keystroke dynamics analysis
 * - Becker (2016) - Compiler error metrics
 * - Perera (2023) - Real-time stress detection
 * 
 * Formula:
 * ANXIETY_SCORE = 0.7×Tier1 + 0.25×Tier2 + 0.05×Tier3 (0-100 scale)
 */
class AnxietyScorer {
public:
    AnxietyScorer();
    ~AnxietyScorer() = default;
    
    /**
     * @brief Calculate the anxiety score from a metrics snapshot.
     * @param metrics Current metrics data
     * @return Anxiety score (0-100)
     */
    double CalculateScore(const MetricsSnapshot& metrics) const;
    
    /**
     * @brief Get the risk level for a given anxiety score.
     * @param score Anxiety score (0-100)
     * @return RiskLevel enum value
     */
    RiskLevel GetRiskLevel(double score) const;
    
    /**
     * @brief Get recommendation message for a risk level.
     * @param level Current risk level
     * @return User-friendly recommendation string
     */
    std::string GetRecommendation(RiskLevel level) const;
    
    /**
     * @brief Check if warning should be shown (respects cooldown).
     * @param level Current risk level
     * @return true if warning is appropriate
     */
    bool ShouldShowWarning(RiskLevel level) const;
    
    /**
     * @brief Reset warning cooldown timer.
     */
    void ResetWarningCooldown();

private:
    // Normalization functions (map raw values to 0-1 scale)
    double NormalizeLatencyVariance(double latencyMs) const;
    double NormalizeTypingSpeed(double wpm) const;
    double NormalizeErrorFrequency(double errorsPerMin) const;
    double NormalizePauseRatio(double ratio) const;
    double NormalizeErrorResolution(double seconds) const;
    double NormalizeBackspaceRate(double rate) const;
    double NormalizeConsecutiveErrors(int count) const;
    double NormalizeUndoRedo(int count) const;
    double NormalizeIdleRatio(double ratio) const;
    double NormalizeFocusSwitches(double perMin) const;
    double NormalizeCompileSuccess(double percent) const;
    double NormalizeFragmentation(double ratio) const;
    
    // Clamp value to [0, 1] range
    double Clamp01(double value) const;

private:
    // Tier weights (from Yu et al. 2025)
    static constexpr double TIER1_WEIGHT = 0.70;
    static constexpr double TIER2_WEIGHT = 0.25;
    static constexpr double TIER3_WEIGHT = 0.05;
    
    // Tier 1 sub-weights (within tier)
    static constexpr double T1_LATENCY_WEIGHT = 0.20;
    static constexpr double T1_SPEED_WEIGHT = 0.20;
    static constexpr double T1_ERROR_FREQ_WEIGHT = 0.20;
    static constexpr double T1_PAUSE_WEIGHT = 0.15;
    static constexpr double T1_RESOLUTION_WEIGHT = 0.15;
    
    // Tier 2 sub-weights
    static constexpr double T2_BACKSPACE_WEIGHT = 0.30;
    static constexpr double T2_CONSEC_ERR_WEIGHT = 0.25;
    static constexpr double T2_UNDO_WEIGHT = 0.25;
    static constexpr double T2_IDLE_WEIGHT = 0.20;
    
    // Tier 3 sub-weights
    static constexpr double T3_FOCUS_WEIGHT = 0.35;
    static constexpr double T3_SUCCESS_WEIGHT = 0.35;
    static constexpr double T3_FRAG_WEIGHT = 0.30;
    
    // Normalization thresholds
    double m_maxLatencyVariance;      // ms
    double m_baselineTypingSpeed;     // WPM
    double m_maxErrorsPerMinute;
    double m_maxPauseRatio;
    double m_maxErrorResolutionSec;
    double m_maxBackspaceRate;
    int m_maxConsecutiveErrors;
    int m_maxUndoRedoCount;
    double m_maxIdleRatio;
    double m_maxFocusSwitchesPerMin;
    double m_maxFragmentation;
    
    // Warning state
    mutable std::chrono::steady_clock::time_point m_lastWarningTime;
    int m_warningCooldownMinutes;
};

} // namespace AnxietyMonitor

#endif // ANXIETY_SCORER_H
