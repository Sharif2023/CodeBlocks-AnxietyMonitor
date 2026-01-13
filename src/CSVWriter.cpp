#include "CSVWriter.h"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#endif

namespace AnxietyMonitor {

// CSV column headers (24 columns as specified)
const std::vector<std::string> CSVWriter::CSV_HEADERS = {
    "timestamp",
    "session_id",
    "project_name",
    "file_path",
    "language",
    "typing_speed_wpm",
    "latency_variance_ms",
    "error_freq_permin",
    "pause_ratio",
    "error_resolution_time",
    "backspace_rate",
    "consecutive_errors",
    "undo_redo_count",
    "idle_ratio",
    "focus_switches",
    "compile_success_rate",
    "session_fragmentation",
    "anxiety_score",
    "risk_level",
    "timestamp_batch",
    "cpu_usage",
    "memory_usage",
    "window_focused",
    "keystrokes_total",
    "compile_attempts",
    "error_count_total"
};

CSVWriter::CSVWriter()
    : m_isSessionActive(false)
    , m_rowsWritten(0)
{
    m_outputDirectory = GetDefaultOutputDirectory();
}

CSVWriter::~CSVWriter()
{
    // Auto-save on destruction (plugin exit)
    if (m_isSessionActive) {
        EndSession();
    }
}

std::string CSVWriter::GetDefaultOutputDirectory()
{
    std::string homeDir;
    
#ifdef _WIN32
    // Windows: Use USERPROFILE or HOMEPATH
    char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        homeDir = userProfile;
    } else {
        char* homeDrive = std::getenv("HOMEDRIVE");
        char* homePath = std::getenv("HOMEPATH");
        if (homeDrive && homePath) {
            homeDir = std::string(homeDrive) + homePath;
        } else {
            homeDir = "C:\\Users\\Default";
        }
    }
    // Convert to forward slashes for consistency
    for (char& c : homeDir) {
        if (c == '\\') c = '/';
    }
    return homeDir + "/.codeblocks/anxiety_monitor/sessions";
#else
    // Unix-like: Use HOME or getpwuid
    char* home = std::getenv("HOME");
    if (home) {
        homeDir = home;
    } else {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            homeDir = pw->pw_dir;
        } else {
            homeDir = "/tmp";
        }
    }
    return homeDir + "/.codeblocks/anxiety_monitor/sessions";
#endif
}

bool CSVWriter::EnsureDirectoryExists(const std::string& path)
{
    // Create directory recursively
    std::string currentPath;
    std::istringstream pathStream(path);
    std::string segment;
    
#ifdef _WIN32
    char delimiter = '/';
    // Handle drive letter on Windows (e.g., "C:")
    if (path.length() >= 2 && path[1] == ':') {
        currentPath = path.substr(0, 2);
        pathStream.str(path.substr(2));
    }
#else
    char delimiter = '/';
    if (!path.empty() && path[0] == '/') {
        currentPath = "/";
    }
#endif
    
    while (std::getline(pathStream, segment, delimiter)) {
        if (segment.empty()) continue;
        
        if (!currentPath.empty() && currentPath.back() != '/' && currentPath.back() != ':') {
            currentPath += '/';
        }
        currentPath += segment;
        
#ifdef _WIN32
        // Convert to Windows path for mkdir
        std::string winPath = currentPath;
        for (char& c : winPath) {
            if (c == '/') c = '\\';
        }
        _mkdir(winPath.c_str());
#else
        mkdir(currentPath.c_str(), 0755);
#endif
    }
    
    // Verify directory exists
#ifdef _WIN32
    std::string winPath = path;
    for (char& c : winPath) {
        if (c == '/') c = '\\';
    }
    DWORD attrs = GetFileAttributesA(winPath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

std::string CSVWriter::GenerateTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm* tm = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

std::string CSVWriter::EscapeCSV(const std::string& value)
{
    // If value contains comma, quote, or newline, wrap in quotes and escape quotes
    bool needsQuotes = false;
    for (char c : value) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needsQuotes = true;
            break;
        }
    }
    
    if (!needsQuotes) {
        return value;
    }
    
    std::string escaped = "\"";
    for (char c : value) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

bool CSVWriter::StartSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Close any existing session
    if (m_isSessionActive) {
        Flush();
        m_file.close();
    }
    
    // Ensure output directory exists
    if (!EnsureDirectoryExists(m_outputDirectory)) {
        return false;
    }
    
    // Generate filename: anxiety_session_YYYYMMDD_HHMMSS.csv
    std::string filename = "anxiety_session_" + GenerateTimestamp() + ".csv";
    
#ifdef _WIN32
    std::string filePath = m_outputDirectory;
    for (char& c : filePath) {
        if (c == '/') c = '\\';
    }
    m_currentFilePath = filePath + "\\" + filename;
#else
    m_currentFilePath = m_outputDirectory + "/" + filename;
#endif
    
    // Open file for writing
    m_file.open(m_currentFilePath, std::ios::out | std::ios::trunc);
    if (!m_file.is_open()) {
        return false;
    }
    
    m_sessionId = sessionId;
    m_isSessionActive = true;
    m_rowsWritten = 0;
    
    // Write header row
    WriteHeader();
    
    return true;
}

void CSVWriter::WriteHeader()
{
    if (!m_file.is_open()) return;
    
    for (size_t i = 0; i < CSV_HEADERS.size(); ++i) {
        if (i > 0) m_file << ",";
        m_file << CSV_HEADERS[i];
    }
    m_file << "\n";
    m_file.flush();  // Immediate flush for header
}

bool CSVWriter::WriteSnapshot(const MetricsSnapshot& snapshot)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_isSessionActive || !m_file.is_open()) {
        return false;
    }
    
    // Write all 24 columns (with additional error_count_total = 26 total per spec)
    m_file << EscapeCSV(snapshot.timestamp) << ","
           << EscapeCSV(snapshot.sessionId) << ","
           << EscapeCSV(snapshot.projectName) << ","
           << EscapeCSV(snapshot.filePath) << ","
           << EscapeCSV(snapshot.language) << ","
           << std::fixed << std::setprecision(2)
           << snapshot.typingSpeedWpm << ","
           << snapshot.latencyVarianceMs << ","
           << snapshot.errorFreqPerMin << ","
           << snapshot.pauseRatio << ","
           << snapshot.errorResolutionTime << ","
           << snapshot.backspaceRate << ","
           << snapshot.consecutiveErrors << ","
           << snapshot.undoRedoCount << ","
           << snapshot.idleRatio << ","
           << snapshot.focusSwitches << ","
           << snapshot.compileSuccessRate << ","
           << snapshot.sessionFragmentation << ","
           << snapshot.anxietyScore << ","
           << EscapeCSV(snapshot.riskLevel) << ","
           << EscapeCSV(snapshot.timestampBatch) << ","
           << snapshot.cpuUsage << ","
           << snapshot.memoryUsage << ","
           << (snapshot.windowFocused ? "true" : "false") << ","
           << snapshot.keystrokesTotal << ","
           << snapshot.compileAttempts << ","
           << snapshot.errorCountTotal
           << "\n";
    
    // Flush immediately for data safety (auto-save behavior)
    m_file.flush();
    
    ++m_rowsWritten;
    return true;
}

void CSVWriter::Flush()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_file.is_open()) {
        m_file.flush();
    }
}

void CSVWriter::EndSession()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_isSessionActive && m_file.is_open()) {
        m_file.flush();
        m_file.close();
    }
    
    m_isSessionActive = false;
}

std::string CSVWriter::GetCurrentFilePath() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentFilePath;
}

void CSVWriter::SetOutputDirectory(const std::string& directory)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_outputDirectory = directory;
}

} // namespace AnxietyMonitor
