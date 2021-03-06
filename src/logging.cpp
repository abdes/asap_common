//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <common/logging.h>

#include <iomanip>  // std::setw
#include <sstream>  // std::ostringstream

#include <common/assert.h>

#if defined _WIN32 && !defined(__cplusplus_winrt)
# include <spdlog/sinks/wincolor_sink.h>
#else
# include <spdlog/sinks/ansicolor_sink.h>
#endif



namespace asap {
namespace logging {

// ---------------------------------------------------------------------------
// Static members initialization
// ---------------------------------------------------------------------------

/// The default logging format
const char *Logger::DEFAULT_LOG_FORMAT =
    "[%Y-%m-%d %T.%e] [%t] [%^%l%$] [%n] %v";

// Synchronization mutex for sinks
std::mutex Registry::sinks_mutex_;
// Synchronization mutex for the loggers collection.
std::recursive_mutex Registry::loggers_mutex_;


// ---------------------------------------------------------------------------
// Logger
// ---------------------------------------------------------------------------

Logger::Logger(std::string name, spdlog::sink_ptr sink) {
  logger_ = std::make_shared<spdlog::logger>(name, sink);
  logger_->set_pattern(DEFAULT_LOG_FORMAT);
  logger_->set_level(spdlog::level::trace);
  // Ensure that critical errors, especially ASSERT/PANIC, get flushed
  logger_->flush_on(spdlog::level::critical);
}

spdlog::logger &Registry::GetLogger(std::string const &name) {
  std::lock_guard<std::recursive_mutex> lock(loggers_mutex_);
  auto &loggers = Loggers();
  auto search = loggers.find(name);
  if (search == loggers.end()) {
    auto new_logger = loggers.emplace(name, Logger(name, delegating_sink()));
    search = new_logger.first;
  }
  return *(search->second.logger_);
}

// ---------------------------------------------------------------------------
// Registry
// ---------------------------------------------------------------------------

void Registry::PushSink(spdlog::sink_ptr sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  auto &sinks = sinks_();
  // Push the current sink on the stack and use the new one
  sinks.emplace(delegating_sink()->SwapSink(sink));
}

void Registry::PopSink() {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  auto &sinks = sinks_();
  ASAP_ASSERT(!sinks.empty() &&
              "call to PopSink() not matching a previous call to PushSink()");
  if (!sinks.empty()) {
    auto &sink = sinks.top();
    // Assign this previous sink to the delegating sink
    delegating_sink()->SwapSink(sink);
    sinks.pop();
  }
}

std::stack<spdlog::sink_ptr> &Registry::sinks_() {
  static std::stack<spdlog::sink_ptr> sinks;
  return sinks;
}

void Registry::SetLogLevel(spdlog::level::level_enum log_level) {
  std::lock_guard<std::recursive_mutex> lock(loggers_mutex_);
  auto &loggers = Loggers();
  for (auto &log : loggers) {
    // Thread safe
    log.second.SetLevel(log_level);
  }
}

void Registry::SetLogFormat(const std::string &log_format) {
  std::lock_guard<std::recursive_mutex> lock(loggers_mutex_);
  auto &loggers = Loggers();
  for (auto &log : loggers) {
    // Not thread safe
    std::lock_guard<std::mutex> log_lock(*log.second.logger_mutex_.get());
    log.second.logger_->set_pattern(log_format);
  }
}

std::unordered_map<std::string, Logger> &Registry::Loggers() {
  static auto &loggers_static = predefined_loggers_();
  return loggers_static;
}

std::unordered_map<std::string, Logger> &Registry::predefined_loggers_() {
  static std::unordered_map<std::string, Logger> all_loggers;
  all_loggers.emplace("misc", Logger("misc", delegating_sink()));
  all_loggers.emplace("testing", Logger("testing", delegating_sink()));
  all_loggers.emplace("main", Logger("main", delegating_sink()));
  return all_loggers;
}

std::shared_ptr<DelegatingSink> &Registry::delegating_sink() {
  static auto sink_static = std::shared_ptr<DelegatingSink>(delegating_sink_());
  return sink_static;
}

DelegatingSink *Registry::delegating_sink_() {
  // Add a default console sink
#if defined _WIN32 && !defined(__cplusplus_winrt)
  auto default_sink =
      std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
#else
  auto default_sink =
      std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif

  static auto sink = new DelegatingSink(default_sink);
  return sink;
}

// ---------------------------------------------------------------------------
// Helper for file name and line number formatting
// ---------------------------------------------------------------------------
#ifndef NDEBUG
/*!
 * @brief Make a string with the soruce code file name and line number at which
 * the log message was produced.
 * @param file source code file name.
 * @param line source code line number.
 * @return a formatted string with the file name and line number.
 */
std::string FormatFileAndLine(char const *file, char const *line) {
  constexpr static int FILE_MAX_LENGTH = 70;
  std::ostringstream ostr;
  std::string fstr(file);
  if (fstr.length() > FILE_MAX_LENGTH) {
    fstr = fstr.substr(0, 7).append("...").append(fstr.substr(
        fstr.length() - FILE_MAX_LENGTH + 10, FILE_MAX_LENGTH - 10));
  }
  ostr << "[" << std::setw(FILE_MAX_LENGTH) << std::right << fstr << ":"
       << std::setw(5) << std::setfill('0') << std::right << line << "] ";
  return ostr.str();
}
#endif // NDEBUG

}  // namespace logging
}  // namespace asap
