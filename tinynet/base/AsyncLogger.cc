// 
// Author       : gan
// Date         : 2022-12
// 
#include <tinynet/base/AsyncLogger.h>
#include <tinynet/base/LogFile.h>

using namespace tinynet;
using namespace std;

AsyncLogger::AsyncLogger(const std::string& basename, int flushInterval) 
  : basename_(basename),
    running_(false),
    latch_(1),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer)
{
  // FIXME : Buffer->bzero? 
  buffers_.reserve(16);
}

void AsyncLogger::start() {
  running_ = true;
  thread_ = std::thread([this]() { this->threadFunc(); });
  latch_.wait();
}

void AsyncLogger::append(const char* logline, int len) {
  unique_lock<mutex> lk(mtx_);
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, len);
  } else {
    buffers_.push_back(std::move(currentBuffer_));
    if (nextBuffer_) {
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      // ask for more buffer
      currentBuffer_.reset(new Buffer);
    }
    currentBuffer_->append(logline, len);
    cond_.notify_one();
  }
}

void AsyncLogger::threadFunc() {
  assert(running_ == true);
  latch_.countDown();
  LogFile output(basename_);
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      unique_lock<mutex> lk(mtx_);
      // wait for the data
      // FIXME :  3s 
      cond_.wait_for(lk, 3s, [this](){ return !buffers_.empty(); });
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }   
    assert(!buffersToWrite.empty());
    if (buffersToWrite.size() > 25) {
      fputs("too much log\n", stderr);
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }
    for (const auto& buffer : buffersToWrite) {
      output.append(buffer->data(), buffer->length());
    }
    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    }
    if (!newBuffer1)
    {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }
    if (!newBuffer2)
    {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }
    buffersToWrite.clear();
    output.flush();    
  }
  output.flush();
}