// 
// Author       : gan
// Date         : 2022-08-23
// 
#ifndef TINYNET_BASE_TIMESTAMP_H_
#define TINYNET_BASE_TIMESTAMP_H_

#include <chrono>

namespace tinynet
{
using namespace std::literals::chrono_literals;

typedef std::chrono::nanoseconds   Nanosecond;
typedef std::chrono::microseconds  Microsecond;
typedef std::chrono::milliseconds  Millisecond;
typedef std::chrono::seconds       Second;
typedef std::chrono::minutes       Minute;
typedef std::chrono::hours         Hour;
typedef std::chrono::time_point<std::chrono::system_clock, Nanosecond> Timestamp;



namespace time
{
inline Timestamp now()
{ return std::chrono::system_clock::now(); }

inline Timestamp nowAfter(Nanosecond interval)
{ return now() + interval; }

inline Timestamp nowBefore(Nanosecond interval)
{ return now() - interval; } 
} // namespace time

} // namespace tinynet

#endif // TINYNET_BASE_TIMESTAMP_H_