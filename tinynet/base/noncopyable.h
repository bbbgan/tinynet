// 
// Author       : gan
// Date         : 2022-08-23
// 
#ifndef NONCOPYABLE_H_
#define NONCOPYABLE_H_

namespace tinynet
{
class noncopyable 
{
  protected:
    noncopyable() = default;
    ~noncopyable() = default;
  public:
    noncopyable(const noncopyable&) = delete;
    const noncopyable& operator=(const noncopyable&) = delete;
};
} // namespace tinynet

#endif  // NONCOPYABL_H_