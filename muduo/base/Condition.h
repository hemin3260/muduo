// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include <muduo/base/Mutex.h>

#include <boost/noncopyable.hpp>
#include <pthread.h>

namespace muduo
{

class Condition : boost::noncopyable
{
 public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex)
  {
    MCHECK(pthread_cond_init(&pcond_, NULL));
  }

  ~Condition()
  {
    MCHECK(pthread_cond_destroy(&pcond_));
  }
  
 //UnassignGuard 表示mutex不属于当前的pid了，当最desctructor时，又会被重新赋值
 //因为wait是有释放锁的过程，它返回的时候，再次锁定
 //局限就是，如果wait之后还有很多操作，会导致没有即使赋值
 //这里的想法是：因为封装了MutexLock类，并且这个类有自己的pid，那么在pthread_cond_wait的时候，会有一个
 //锁定和释放的过程。这个过程是已经封装了的，怎么让这个过程去修改pid。因此引入了一个unassignGuard的中间类
 
  void wait()
  {
    MutexLock::UnassignGuard ug(mutex_);
    //友类，所以可以访问
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(double seconds);

  void notify()
  {
    MCHECK(pthread_cond_signal(&pcond_));
  }

  void notifyAll()
  {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
};

}
#endif  // MUDUO_BASE_CONDITION_H
