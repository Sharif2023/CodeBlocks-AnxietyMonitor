# Anxiety Monitor - Code::Blocks Plugin

A research-backed behavioral analysis plugin for Code::Blocks IDE that monitors 12 validated metrics during coding sessions and displays a live anxiety score (0-100) with risk levels.

## Features

- **12 Research-Validated Metrics** - Based on studies by Lau (2018), Yu et al. (2025), Becker (2016), and Perera (2023)
- **Live Anxiety Scoring** - Real-time 0-100 scale with 4 risk levels (🟢 LOW, 🟡 MODERATE, 🟠 HIGH, 🔴 CRITICAL)
- **Auto-Save CSV** - Data automatically saved every 30 seconds
- **Auto-Export on Exit** - Sessions are preserved when stopping or closing Code::Blocks
- **Non-Intrusive UI** - Status bar updates without popup interruptions
- **24-Column CSV Export** - Comprehensive data for research analysis

## Monitored Metrics

### Tier 1 - High Predictive (70% weight)
1. Keystroke Latency Variance (ms)
2. Typing Speed Deviation (WPM from baseline)
3. Error Frequency (errors/min)
4. Pause Duration Ratio (>2s gaps)
5. Error Resolution Time (seconds)

### Tier 2 - Medium Predictive (25% weight)
6. Backspace Rate (per 100 keystrokes)
7. Consecutive Errors (last 5 minutes)
8. Undo/Redo Frequency (per 10 min)
9. Idle Time Ratio (%)

### Tier 3 - Low Predictive (5% weight)
10. Focus Switches (tab changes/min)
11. Compile Success Rate (%)
12. Session Fragmentation (breaks >30s)

## Installation

### Installation (Easiest Way)

1. **Download** the `AnxietyMonitor.cbplugin` file.
2. Open **Code::Blocks**.
3. Go to `Plugins` -> `Manage plugins...`
4. Click `Install new`.
5. Select `AnxietyMonitor.cbplugin`.
6. Follow the prompts and **Restart Code::Blocks**.

### Manual Installation (If above fails)

1. Rename `AnxietyMonitor.cbplugin` to `AnxietyMonitor.zip`.
2. Extract it to get `AnxietyMonitor.dll` and `AnxietyMonitor.zip` (inner zip).
3. Copy **BOTH** files to your Code::Blocks plugins folder:
   - `C:\Program Files\CodeBlocks\share\CodeBlocks\plugins\`
4. Restart Code::Blocks.

### Troubleshooting

**Plugin not loading?**
- **Missing DLLs**: Ensure standard MinGW runtime DLLs (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) are in your Code::Blocks directory or system PATH.
- **Architecture**: This plugin is 64-bit. Ensure you are using 64-bit Code::Blocks (check `Help -> About`).


## Usage

1. **Start Session**: Click the ▶️ Start button in toolbar or use `Plugins > Anxiety Monitor > Start Session`
2. **Monitor**: Status bar shows live metrics: `[Anxiety Monitor] Score: 24 [OK] | Errors: 2/min | Typing: 45wpm`
3. **Pause/Resume**: Click ⏸ Pause to temporarily stop monitoring (data is auto-saved)
4. **Export**: Click 💾 Export to save a copy of the session CSV
5. **End Session**: Click Stop or use menu - CSV is automatically saved

## CSV Output

Sessions are saved to:
- **Windows**: `%USERPROFILE%\.codeblocks\anxiety_monitor\sessions\`
- **Linux**: `~/.codeblocks/anxiety_monitor/sessions/`

Filename format: `anxiety_session_YYYYMMDD_HHMMSS.csv`

### CSV Columns (24 total)
```
timestamp, session_id, project_name, file_path, language,
typing_speed_wpm, latency_variance_ms, error_freq_permin,
pause_ratio, error_resolution_time, backspace_rate, consecutive_errors,
undo_redo_count, idle_ratio, focus_switches, compile_success_rate,
session_fragmentation, anxiety_score, risk_level, timestamp_batch,
cpu_usage, memory_usage, window_focused, keystrokes_total,
compile_attempts, error_count_total
```

## Risk Levels

| Level | Score | Meaning |
|-------|-------|---------|
| 🟢 LOW | 0-30 | Normal coding flow |
| 🟡 MODERATE | 31-60 | Elevated stress, consider a break |
| 🟠 HIGH | 61-80 | Significant anxiety, intervention suggested |
| 🔴 CRITICAL | 81-100 | Acute distress, immediate support recommended |

## Configuration

Default settings (user-friendly, non-intrusive):
- Update interval: 30 seconds
- Popup warnings: Disabled
- Sound alerts: Disabled
- Auto-save on exit: Enabled

## Research References

- **Lau (2018)** - Keystroke dynamics for stress detection (89.5% accuracy)
- **Yu et al. (2025)** - Programming anxiety ML model (97.8% accuracy)
- **Becker (2016)** - Compiler error metrics correlation
- **Perera (2023)** - Real-time stress detection systems (83.6% accuracy)

## Privacy

This plugin does **NOT** log any code content. Only behavioral metrics (timing, counts, frequencies) are recorded. All data stays local on your machine.

## License

MIT License - Free for academic and commercial use.

## Support

For issues or feature requests, please open an issue on the project repository.