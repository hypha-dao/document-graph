#include "logger/logger.hpp"

namespace hypha
{

Logger& 
Logger::instance() 
{
  static Logger l;
  return l;
}

void 
Logger::pushMessage(std::string message) 
{
  m_log.push_back(std::move(message));
}

void 
Logger::popTrace() 
{
  m_trace.pop_back();
}

void 
Logger::pushTrace(std::string trace) 
{
  m_trace.push_back(std::move(trace));
}

std::string 
Logger::generateMessage() 
{
  std::string ss = "\n------------------- Stack Trace -------------------\n";
  std::string tab = "";
  for (auto& message : m_trace) {
    ss += "\n" + tab + message;
    tab += "\t";
  }

  ss += "\n\n--------------------------------------------------\n";

  ss += "\n------------------- Log -------------------\n";

  for (auto& message : m_log) {
    ss += "\n" + message;
  }

  ss += "\n\n--------------------------------------------------\n";

  return ss;
}



}