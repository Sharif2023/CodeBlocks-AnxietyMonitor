#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include "MetricsData.h"

namespace AnxietyMonitor {

/**
 * @class CSVWriter
 * @brief Handles CSV file creation and data persistence for anxiety monitoring sessions.
 * 
 * Features:
 * - Auto-creates session files with timestamp-based naming
 * - Real-time append every 30 seconds
 * - Auto-save on session stop or plugin exit
 * - Thread-safe write operations
 * - 24-column CSV format per research specifications
 */
class CSVWriter {
public:
    CSVWriter();
    ~CSVWriter();
    
    /**
     * @brief Initialize a new session CSV file.
     * @param sessionId Unique session identifier
     * @return true if file was created successfully
     */
    bool StartSession(const std::string& sessionId);
    
    /**
     * @brief Write a metrics snapshot to the current session file.
     * @param snapshot The metrics data to write
     * @return true if write was successful
     */
    bool WriteSnapshot(const MetricsSnapshot& snapshot);
    
    /**
     * @brief Force flush all buffered data to disk.
     * Called on pause, stop, or exit for auto-save functionality.
     */
    void Flush();
    
    /**
     * @brief End the current session and close the file.
     * Automatically flushes any remaining data.
     */
    void EndSession();
    
    /**
     * @brief Get the current session file path.
     * @return Full path to the active CSV file, or empty if no session
     */
    std::string GetCurrentFilePath() const;
    
    /**
     * @brief Set the output directory for CSV files.
     * @param directory The directory path (will be created if needed)
     */
    void SetOutputDirectory(const std::string& directory);
    
    /**
     * @brief Get the default output directory.
     * @return Path to ~/.codeblocks/anxiety_monitor/sessions/
     */
    static std::string GetDefaultOutputDirectory();
    
    /**
     * @brief Check if a session is currently active.
     */
    bool IsSessionActive() const { return m_isSessionActive; }
    
    /**
     * @brief Get total rows written in current session.
     */
    int GetRowsWritten() const { return m_rowsWritten; }

private:
    /**
     * @brief Write the CSV header row.
     */
    void WriteHeader();
    
    /**
     * @brief Create the output directory if it doesn't exist.
     */
    bool EnsureDirectoryExists(const std::string& path);
    
    /**
     * @brief Generate timestamp string for filenames.
     */
    static std::string GenerateTimestamp();
    
    /**
     * @brief Escape a string value for CSV format.
     */
    static std::string EscapeCSV(const std::string& value);

private:
    std::ofstream m_file;
    std::string m_outputDirectory;
    std::string m_currentFilePath;
    std::string m_sessionId;
    bool m_isSessionActive;
    int m_rowsWritten;
    mutable std::mutex m_mutex;
    
    // CSV column headers (24 columns)
    static const std::vector<std::string> CSV_HEADERS;
};

} // namespace AnxietyMonitor

#endif // CSV_WRITER_H
