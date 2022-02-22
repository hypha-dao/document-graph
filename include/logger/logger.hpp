#pragma once

#include <vector>
#include <string>

#include <eosio/eosio.hpp>

#include "document_graph/util.hpp"

namespace hypha
{
  
class Logger
{
  public:

  static Logger&
  instance();

  void 
  pushTrace(std::string trace);

  void
  popTrace();

  void
  pushMessage(std::string message);

  std::string
  generateMessage();

  private:

  std::vector<std::string> m_trace;
  std::vector<std::string> m_log;
};

#ifdef USE_LOGGING
#include "boost_current_function.hpp"

class AutoTraceDroper 
{
  public:
  AutoTraceDroper() {}

  ~AutoTraceDroper() { Logger::instance().popTrace(); }
};

#define TRACE_FUNCTION() AutoTraceDroper autoDrop##__LINE__{}; Logger::instance().pushTrace(util::to_str(__FILE__, " : ", BOOST_CURRENT_FUNCTION));
#define TRACE_ERROR(message) AutoTraceDroper autoDrop##__LINE__{}; Logger::instance().pushTrace(util::to_str(__FILE__, ":", __LINE__, ": ", message));
#define LOG_MESSAGE(message) Logger::instance().pushMessage(util::to_str("[DEBUG]: ", __FILE__, ":", __LINE__, ": ", message));
#define EOS_CHECK(condition, message)\
{\
if (!(condition)) {\
  TRACE_ERROR(message)\
  eosio::check(false, Logger::instance().generateMessage());\
}\
}
#else
#define TRACE_FUNCTION()
#define TRACE_ERROR(message)
#define LOG_MESSAGE(message)
#define EOS_CHECK(condition, message)\
{\
if (!(condition)) {\
  eosio::check(false, message);\
}\
}
#endif 

}

