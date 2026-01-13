#include "UIComponents.h"
#include <wx/datetime.h>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace AnxietyMonitor {

// ============================================================================
// AnxietyPanel Implementation
// ============================================================================

AnxietyPanel::AnxietyPanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id, wxDefaultPosition, wxSize(300, 400))
    , m_sessionState(SessionState::STOPPED)
{
    SetBackgroundColour(wxColour(45, 45, 48));  // Dark theme
    CreateControls();
}

AnxietyPanel::~AnxietyPanel()
{
}

void AnxietyPanel::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    wxStaticText* titleLabel = new wxStaticText(this, wxID_ANY, 
        "ANXIETY MONITOR - Live Metrics");
    titleLabel->SetForegroundColour(wxColour(200, 200, 200));
    wxFont titleFont = titleLabel->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLabel->SetFont(titleFont);
    mainSizer->Add(titleLabel, 0, wxALL | wxALIGN_CENTER, 10);
    
    // Separator
    mainSizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    
    // Score section
    wxBoxSizer* scoreSizer = new wxBoxSizer(wxHORIZONTAL);
    
    m_scoreLabel = new wxStaticText(this, wxID_ANY, "Score:");
    m_scoreLabel->SetForegroundColour(wxColour(180, 180, 180));
    scoreSizer->Add(m_scoreLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    m_scoreValue = new wxStaticText(this, wxID_ANY, "0");
    wxFont scoreFont = m_scoreValue->GetFont();
    scoreFont.SetPointSize(24);
    scoreFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_scoreValue->SetFont(scoreFont);
    m_scoreValue->SetForegroundColour(wxColour(76, 175, 80));  // Green
    scoreSizer->Add(m_scoreValue, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    m_riskLabel = new wxStaticText(this, wxID_ANY, "LOW");
    m_riskLabel->SetForegroundColour(wxColour(76, 175, 80));
    scoreSizer->Add(m_riskLabel, 0, wxALIGN_CENTER_VERTICAL);
    
    mainSizer->Add(scoreSizer, 0, wxALL | wxALIGN_CENTER, 10);
    
    // Score gauge
    m_scoreGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, 
        wxSize(-1, 10), wxGA_HORIZONTAL | wxGA_SMOOTH);
    m_scoreGauge->SetValue(0);
    mainSizer->Add(m_scoreGauge, 0, wxEXPAND | wxLEFT | wxRIGHT, 15);
    
    // Trend display
    m_trendLabel = new wxStaticText(this, wxID_ANY, "Trend: --");
    m_trendLabel->SetForegroundColour(wxColour(150, 150, 150));
    mainSizer->Add(m_trendLabel, 0, wxALL, 10);
    
    mainSizer->AddSpacer(10);
    
    // Current Metrics section
    wxStaticText* metricsTitle = new wxStaticText(this, wxID_ANY, "CURRENT METRICS");
    metricsTitle->SetForegroundColour(wxColour(120, 180, 255));
    mainSizer->Add(metricsTitle, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    
    // Metrics grid
    wxFlexGridSizer* metricsSizer = new wxFlexGridSizer(2, 5, 5);
    metricsSizer->AddGrowableCol(1, 1);
    
    m_errorsLabel = new wxStaticText(this, wxID_ANY, "Errors/min:    0.0");
    m_errorsLabel->SetForegroundColour(wxColour(180, 180, 180));
    metricsSizer->Add(m_errorsLabel, 0, wxEXPAND);
    
    m_typingLabel = new wxStaticText(this, wxID_ANY, "Typing speed:  0 WPM");
    m_typingLabel->SetForegroundColour(wxColour(180, 180, 180));
    metricsSizer->Add(m_typingLabel, 0, wxEXPAND);
    
    m_pauseLabel = new wxStaticText(this, wxID_ANY, "Pause ratio:   0%");
    m_pauseLabel->SetForegroundColour(wxColour(180, 180, 180));
    metricsSizer->Add(m_pauseLabel, 0, wxEXPAND);
    
    m_backspaceLabel = new wxStaticText(this, wxID_ANY, "Backspaces:    0%");
    m_backspaceLabel->SetForegroundColour(wxColour(180, 180, 180));
    metricsSizer->Add(m_backspaceLabel, 0, wxEXPAND);
    
    mainSizer->Add(metricsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 15);
    
    mainSizer->AddSpacer(15);
    
    // Recent Events section
    wxStaticText* eventsTitle = new wxStaticText(this, wxID_ANY, "RECENT EVENTS");
    eventsTitle->SetForegroundColour(wxColour(120, 180, 255));
    mainSizer->Add(eventsTitle, 0, wxLEFT | wxRIGHT, 10);
    
    m_eventsList = new wxListBox(this, wxID_ANY, wxDefaultPosition, 
        wxSize(-1, 120), 0, nullptr, wxLB_SINGLE | wxBORDER_NONE);
    m_eventsList->SetBackgroundColour(wxColour(35, 35, 38));
    m_eventsList->SetForegroundColour(wxColour(180, 180, 180));
    mainSizer->Add(m_eventsList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // Recommendations (initially hidden)
    m_recommendationLabel = new wxStaticText(this, wxID_ANY, "");
    m_recommendationLabel->SetForegroundColour(wxColour(255, 193, 7));  // Amber
    m_recommendationLabel->Wrap(280);
    m_recommendationLabel->Hide();
    mainSizer->Add(m_recommendationLabel, 0, wxALL | wxEXPAND, 10);
    
    SetSizer(mainSizer);
}

void AnxietyPanel::UpdateMetrics(const MetricsSnapshot& snapshot)
{
    RiskLevel level = RiskLevel::LOW;
    if (snapshot.anxietyScore <= 30) level = RiskLevel::LOW;
    else if (snapshot.anxietyScore <= 60) level = RiskLevel::MODERATE;
    else if (snapshot.anxietyScore <= 80) level = RiskLevel::HIGH;
    else level = RiskLevel::CRITICAL;
    
    UpdateScoreDisplay(snapshot.anxietyScore, level);
    UpdateMetricsDisplay(snapshot);
    UpdateTrend(snapshot.anxietyScore);
}

void AnxietyPanel::UpdateScoreDisplay(double score, RiskLevel level)
{
    m_scoreValue->SetLabel(wxString::Format("%.0f", score));
    m_scoreGauge->SetValue(static_cast<int>(score));
    
    wxColour color = GetRiskColor(level);
    m_scoreValue->SetForegroundColour(color);
    m_riskLabel->SetForegroundColour(color);
    m_riskLabel->SetLabel(GetRiskLevelLabel(level));
    
    // Show/hide recommendations based on level
    if (level >= RiskLevel::HIGH) {
        AnxietyScorer scorer;
        m_recommendationLabel->SetLabel(scorer.GetRecommendation(level));
        m_recommendationLabel->Show();
    } else {
        m_recommendationLabel->Hide();
    }
    
    Layout();
    Refresh();
}

void AnxietyPanel::UpdateMetricsDisplay(const MetricsSnapshot& snapshot)
{
    wxColour normalColor(180, 180, 180);
    wxColour warningColor(255, 193, 7);
    wxColour alertColor(244, 67, 54);
    
    // Errors per minute
    wxColour errColor = normalColor;
    if (snapshot.errorFreqPerMin > 5) errColor = alertColor;
    else if (snapshot.errorFreqPerMin > 2) errColor = warningColor;
    m_errorsLabel->SetLabel(wxString::Format("Errors/min:    %.1f", snapshot.errorFreqPerMin));
    m_errorsLabel->SetForegroundColour(errColor);
    
    // Typing speed
    m_typingLabel->SetLabel(wxString::Format("Typing speed:  %.0f WPM", snapshot.typingSpeedWpm));
    
    // Pause ratio
    wxColour pauseColor = normalColor;
    if (snapshot.pauseRatio > 0.4) pauseColor = warningColor;
    m_pauseLabel->SetLabel(wxString::Format("Pause ratio:   %.0f%%", snapshot.pauseRatio * 100));
    m_pauseLabel->SetForegroundColour(pauseColor);
    
    // Backspace rate
    wxColour bsColor = normalColor;
    if (snapshot.backspaceRate > 15) bsColor = warningColor;
    m_backspaceLabel->SetLabel(wxString::Format("Backspaces:    %.1f%%", snapshot.backspaceRate));
    m_backspaceLabel->SetForegroundColour(bsColor);
}

void AnxietyPanel::UpdateTrend(double score)
{
    m_trendData.push_back(score);
    if (m_trendData.size() > MAX_TREND_POINTS) {
        m_trendData.erase(m_trendData.begin());
    }
    
    UpdateTrendDisplay();
}

void AnxietyPanel::UpdateTrendDisplay()
{
    if (m_trendData.empty()) {
        m_trendLabel->SetLabel("Trend: --");
        return;
    }
    
    // Build simple ASCII trend sparkline
    std::string trend = "Trend: ";
    for (size_t i = 0; i < m_trendData.size(); ++i) {
        double score = m_trendData[i];
        if (score < 20) trend += "_";
        else if (score < 40) trend += ".";
        else if (score < 60) trend += "-";
        else if (score < 80) trend += "^";
        else trend += "!";
    }
    
    // Add direction indicator
    if (m_trendData.size() >= 2) {
        double last = m_trendData.back();
        double prev = m_trendData[m_trendData.size() - 2];
        if (last > prev + 5) trend += " [UP]";
        else if (last < prev - 5) trend += " [DOWN]";
        else trend += " [STABLE]";
    }
    
    m_trendLabel->SetLabel(trend);
}

wxColour AnxietyPanel::GetRiskColor(RiskLevel level) const
{
    switch (level) {
        case RiskLevel::LOW:      return wxColour(76, 175, 80);   // Green
        case RiskLevel::MODERATE: return wxColour(255, 193, 7);   // Yellow/Amber
        case RiskLevel::HIGH:     return wxColour(255, 152, 0);   // Orange
        case RiskLevel::CRITICAL: return wxColour(244, 67, 54);   // Red
        default:                  return wxColour(180, 180, 180);
    }
}

void AnxietyPanel::AddEvent(const std::string& event)
{
    std::string timeStr = GetTimeString();
    std::string fullEvent = timeStr + " " + event;
    
    m_eventsList->Insert(fullEvent, 0);
    
    // Keep list manageable
    while (m_eventsList->GetCount() > MAX_EVENTS) {
        m_eventsList->Delete(m_eventsList->GetCount() - 1);
    }
}

void AnxietyPanel::ClearEvents()
{
    m_eventsList->Clear();
}

void AnxietyPanel::SetSessionState(SessionState state)
{
    m_sessionState = state;
    
    if (state == SessionState::STOPPED) {
        m_scoreValue->SetLabel("--");
        m_riskLabel->SetLabel("STOPPED");
        m_riskLabel->SetForegroundColour(wxColour(128, 128, 128));
        m_scoreGauge->SetValue(0);
    } else if (state == SessionState::PAUSED) {
        m_riskLabel->SetLabel("PAUSED");
        m_riskLabel->SetForegroundColour(wxColour(100, 149, 237));  // Cornflower blue
    }
    
    Refresh();
}

std::string AnxietyPanel::GetTimeString() const
{
    wxDateTime now = wxDateTime::Now();
    return now.Format("%H:%M").ToStdString();
}

// ============================================================================
// StatusBarManager Implementation
// ============================================================================

StatusBarManager::StatusBarManager()
{
}

std::string StatusBarManager::FormatStatusText(const MetricsSnapshot& snapshot) const
{
    RiskLevel level = RiskLevel::LOW;
    if (snapshot.anxietyScore <= 30) level = RiskLevel::LOW;
    else if (snapshot.anxietyScore <= 60) level = RiskLevel::MODERATE;
    else if (snapshot.anxietyScore <= 80) level = RiskLevel::HIGH;
    else level = RiskLevel::CRITICAL;
    
    std::ostringstream oss;
    oss << "[Anxiety Monitor] "
        << "Score: " << static_cast<int>(snapshot.anxietyScore) 
        << " " << GetRiskLevelEmoji(level)
        << " | Errors: " << std::fixed << std::setprecision(1) << snapshot.errorFreqPerMin << "/min"
        << " | Typing: " << static_cast<int>(snapshot.typingSpeedWpm) << "wpm"
        << FormatSpeedChange(snapshot.typingSpeedWpm, 40.0)
        << " | Pauses: " << static_cast<int>(snapshot.pauseRatio * 100) << "%";
    
    return oss.str();
}

std::string StatusBarManager::FormatShortStatus(double score, RiskLevel level) const
{
    std::ostringstream oss;
    oss << "Anxiety: " << static_cast<int>(score) << " " << GetRiskLevelEmoji(level);
    return oss.str();
}

std::string StatusBarManager::FormatSpeedChange(double currentWpm, double baselineWpm) const
{
    if (baselineWpm <= 0) return "";
    
    double change = ((currentWpm - baselineWpm) / baselineWpm) * 100;
    
    std::ostringstream oss;
    if (change > 5) {
        oss << " [+" << static_cast<int>(change) << "%]";
    } else if (change < -5) {
        oss << " [" << static_cast<int>(change) << "%]";
    }
    return oss.str();
}

// ============================================================================
// Notification Function
// ============================================================================

void ShowAnxietyNotification(wxWindow* parent, RiskLevel level, const std::string& message)
{
    // Only show notifications for HIGH and CRITICAL
    // This is non-intrusive - only important alerts
    if (level < RiskLevel::HIGH) {
        return;
    }
    
    wxString title;
    long style = wxOK | wxICON_INFORMATION;
    
    switch (level) {
        case RiskLevel::HIGH:
            title = "Anxiety Monitor - Take a Moment";
            style = wxOK | wxICON_WARNING;
            break;
        case RiskLevel::CRITICAL:
            title = "Anxiety Monitor - Please Take a Break";
            style = wxOK | wxICON_EXCLAMATION;
            break;
        default:
            return;
    }
    
    // Use a modeless message to not block the user
    wxMessageBox(message, title, style, parent);
}

} // namespace AnxietyMonitor
