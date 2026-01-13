#include <cassert>
#include <iostream>
#include <cmath>
#include <string>

// Include the headers we want to test
#include "../src/MetricsData.h"
#include "../src/AnxietyScorer.h"

using namespace AnxietyMonitor;

// ============================================================================
// Test Utilities
// ============================================================================

int testsRun = 0;
int testsPassed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    std::cout << "Running " << #name << "... "; \
    testsRun++; \
    try { \
        name(); \
        testsPassed++; \
        std::cout << "PASSED" << std::endl; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << std::endl; \
    } \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Expected " + std::to_string(expected) + " but got " + std::to_string(actual)); \
    } \
} while(0)

#define ASSERT_NEAR(expected, actual, tolerance) do { \
    if (std::abs((expected) - (actual)) > (tolerance)) { \
        throw std::runtime_error("Expected ~" + std::to_string(expected) + " but got " + std::to_string(actual)); \
    } \
} while(0)

#define ASSERT_TRUE(condition) do { \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    } \
} while(0)

// ============================================================================
// Risk Level Tests
// ============================================================================

TEST(test_risk_level_low)
{
    AnxietyScorer scorer;
    ASSERT_EQ(RiskLevel::LOW, scorer.GetRiskLevel(0.0));
    ASSERT_EQ(RiskLevel::LOW, scorer.GetRiskLevel(15.0));
    ASSERT_EQ(RiskLevel::LOW, scorer.GetRiskLevel(30.0));
}

TEST(test_risk_level_moderate)
{
    AnxietyScorer scorer;
    ASSERT_EQ(RiskLevel::MODERATE, scorer.GetRiskLevel(31.0));
    ASSERT_EQ(RiskLevel::MODERATE, scorer.GetRiskLevel(45.0));
    ASSERT_EQ(RiskLevel::MODERATE, scorer.GetRiskLevel(60.0));
}

TEST(test_risk_level_high)
{
    AnxietyScorer scorer;
    ASSERT_EQ(RiskLevel::HIGH, scorer.GetRiskLevel(61.0));
    ASSERT_EQ(RiskLevel::HIGH, scorer.GetRiskLevel(70.0));
    ASSERT_EQ(RiskLevel::HIGH, scorer.GetRiskLevel(80.0));
}

TEST(test_risk_level_critical)
{
    AnxietyScorer scorer;
    ASSERT_EQ(RiskLevel::CRITICAL, scorer.GetRiskLevel(81.0));
    ASSERT_EQ(RiskLevel::CRITICAL, scorer.GetRiskLevel(90.0));
    ASSERT_EQ(RiskLevel::CRITICAL, scorer.GetRiskLevel(100.0));
}

// ============================================================================
// Anxiety Score Calculation Tests
// ============================================================================

TEST(test_score_zero_metrics)
{
    // All metrics at ideal values should give low anxiety
    MetricsSnapshot snapshot;
    snapshot.typingSpeedWpm = 50.0;         // Above baseline (40)
    snapshot.latencyVarianceMs = 0.0;       // No variance
    snapshot.errorFreqPerMin = 0.0;         // No errors
    snapshot.pauseRatio = 0.0;              // No pauses
    snapshot.errorResolutionTime = 0.0;     // Fast resolution
    snapshot.backspaceRate = 0.0;           // No backspaces
    snapshot.consecutiveErrors = 0;
    snapshot.undoRedoCount = 0;
    snapshot.idleRatio = 0.0;
    snapshot.focusSwitches = 0.0;
    snapshot.compileSuccessRate = 100.0;    // All compiles succeed
    snapshot.sessionFragmentation = 0.0;
    
    AnxietyScorer scorer;
    double score = scorer.CalculateScore(snapshot);
    
    ASSERT_TRUE(score >= 0.0);
    ASSERT_TRUE(score <= 30.0);  // Should be LOW
}

TEST(test_score_high_anxiety_metrics)
{
    // All metrics at high-anxiety values
    MetricsSnapshot snapshot;
    snapshot.typingSpeedWpm = 15.0;         // Well below baseline
    snapshot.latencyVarianceMs = 500.0;     // Max variance
    snapshot.errorFreqPerMin = 10.0;        // Max errors
    snapshot.pauseRatio = 0.5;              // 50% pauses
    snapshot.errorResolutionTime = 300.0;   // 5 minutes to fix
    snapshot.backspaceRate = 20.0;          // 20% backspaces
    snapshot.consecutiveErrors = 10;
    snapshot.undoRedoCount = 30;
    snapshot.idleRatio = 0.6;
    snapshot.focusSwitches = 5.0;
    snapshot.compileSuccessRate = 0.0;      // All compiles fail
    snapshot.sessionFragmentation = 0.3;
    
    AnxietyScorer scorer;
    double score = scorer.CalculateScore(snapshot);
    
    ASSERT_TRUE(score >= 70.0);  // Should be HIGH or CRITICAL
}

TEST(test_score_moderate_metrics)
{
    // Moderate values
    MetricsSnapshot snapshot;
    snapshot.typingSpeedWpm = 30.0;
    snapshot.latencyVarianceMs = 150.0;
    snapshot.errorFreqPerMin = 3.0;
    snapshot.pauseRatio = 0.2;
    snapshot.errorResolutionTime = 60.0;
    snapshot.backspaceRate = 8.0;
    snapshot.consecutiveErrors = 3;
    snapshot.undoRedoCount = 10;
    snapshot.idleRatio = 0.2;
    snapshot.focusSwitches = 1.5;
    snapshot.compileSuccessRate = 60.0;
    snapshot.sessionFragmentation = 0.1;
    
    AnxietyScorer scorer;
    double score = scorer.CalculateScore(snapshot);
    
    ASSERT_TRUE(score >= 20.0 && score <= 60.0);  // Should be LOW to MODERATE
}

// ============================================================================
// Tier Weight Tests
// ============================================================================

TEST(test_tier1_dominates)
{
    // Tier 1 has 70% weight, so high Tier 1 values should dominate
    MetricsSnapshot snapshot;
    
    // Tier 1 at max (high anxiety)
    snapshot.typingSpeedWpm = 0.0;
    snapshot.latencyVarianceMs = 500.0;
    snapshot.errorFreqPerMin = 10.0;
    snapshot.pauseRatio = 0.5;
    snapshot.errorResolutionTime = 300.0;
    
    // Tier 2 & 3 at ideal (low anxiety)
    snapshot.backspaceRate = 0.0;
    snapshot.consecutiveErrors = 0;
    snapshot.undoRedoCount = 0;
    snapshot.idleRatio = 0.0;
    snapshot.focusSwitches = 0.0;
    snapshot.compileSuccessRate = 100.0;
    snapshot.sessionFragmentation = 0.0;
    
    AnxietyScorer scorer;
    double score = scorer.CalculateScore(snapshot);
    
    // Should be around 70% of max (due to Tier 1 weight)
    ASSERT_TRUE(score >= 50.0);
}

// ============================================================================
// Rolling Buffer Tests
// ============================================================================

TEST(test_rolling_buffer_mean)
{
    RollingBuffer<double> buffer(5);
    buffer.push(10.0);
    buffer.push(20.0);
    buffer.push(30.0);
    buffer.push(40.0);
    buffer.push(50.0);
    
    double mean = buffer.mean();
    ASSERT_NEAR(30.0, mean, 0.01);
}

TEST(test_rolling_buffer_stddev)
{
    RollingBuffer<double> buffer(5);
    buffer.push(2.0);
    buffer.push(4.0);
    buffer.push(4.0);
    buffer.push(4.0);
    buffer.push(5.0);
    buffer.push(5.0);
    buffer.push(7.0);
    buffer.push(9.0);
    
    // Only last 5 kept: 4, 5, 5, 7, 9
    double stddev = buffer.stddev();
    ASSERT_TRUE(stddev > 1.5 && stddev < 2.5);
}

TEST(test_rolling_buffer_overflow)
{
    RollingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);  // Should remove 1
    buffer.push(5);  // Should remove 2
    
    ASSERT_EQ(3, buffer.size());
    ASSERT_NEAR(4.0, buffer.mean(), 0.01);  // Mean of 3, 4, 5
}

// ============================================================================
// Risk Level Label Tests
// ============================================================================

TEST(test_risk_level_labels)
{
    ASSERT_TRUE(std::string(GetRiskLevelLabel(RiskLevel::LOW)) == "LOW");
    ASSERT_TRUE(std::string(GetRiskLevelLabel(RiskLevel::MODERATE)) == "MODERATE");
    ASSERT_TRUE(std::string(GetRiskLevelLabel(RiskLevel::HIGH)) == "HIGH");
    ASSERT_TRUE(std::string(GetRiskLevelLabel(RiskLevel::CRITICAL)) == "CRITICAL");
}

// ============================================================================
// Recommendation Tests
// ============================================================================

TEST(test_recommendations_exist)
{
    AnxietyScorer scorer;
    
    std::string lowRec = scorer.GetRecommendation(RiskLevel::LOW);
    std::string modRec = scorer.GetRecommendation(RiskLevel::MODERATE);
    std::string highRec = scorer.GetRecommendation(RiskLevel::HIGH);
    std::string critRec = scorer.GetRecommendation(RiskLevel::CRITICAL);
    
    ASSERT_TRUE(!lowRec.empty());
    ASSERT_TRUE(!modRec.empty());
    ASSERT_TRUE(!highRec.empty());
    ASSERT_TRUE(!critRec.empty());
}

// ============================================================================
// Main
// ============================================================================

int main()
{
    std::cout << "==================================" << std::endl;
    std::cout << " Anxiety Monitor Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;
    
    // Risk Level Tests
    RUN_TEST(test_risk_level_low);
    RUN_TEST(test_risk_level_moderate);
    RUN_TEST(test_risk_level_high);
    RUN_TEST(test_risk_level_critical);
    
    // Score Calculation Tests
    RUN_TEST(test_score_zero_metrics);
    RUN_TEST(test_score_high_anxiety_metrics);
    RUN_TEST(test_score_moderate_metrics);
    RUN_TEST(test_tier1_dominates);
    
    // Rolling Buffer Tests
    RUN_TEST(test_rolling_buffer_mean);
    RUN_TEST(test_rolling_buffer_stddev);
    RUN_TEST(test_rolling_buffer_overflow);
    
    // Other Tests
    RUN_TEST(test_risk_level_labels);
    RUN_TEST(test_recommendations_exist);
    
    std::cout << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << " Results: " << testsPassed << "/" << testsRun << " passed" << std::endl;
    std::cout << "==================================" << std::endl;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
