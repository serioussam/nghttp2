/*
 * nghttp2 - HTTP/2 C Library
 *
 * Copyright (c) 2015 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "shrpx_signal.h"

#include <cerrno>

#include "template.h"

using namespace nghttp2;

namespace shrpx {

int shrpx_signal_block_all(sigset_t *oldset) {
  sigset_t newset;

  sigfillset(&newset);

#ifndef NOTHREADS
  int rv;

  rv = pthread_sigmask(SIG_SETMASK, &newset, oldset);

  if (rv != 0) {
    errno = rv;
    return -1;
  }

  return 0;
#else  // NOTHREADS
  return sigprocmask(SIG_SETMASK, &newset, oldset);
#endif // NOTHREADS
}

int shrpx_signal_unblock_all() {
  sigset_t newset;

  sigemptyset(&newset);

#ifndef NOTHREADS
  int rv;

  rv = pthread_sigmask(SIG_SETMASK, &newset, nullptr);

  if (rv != 0) {
    errno = rv;
    return -1;
  }

  return 0;
#else  // NOTHREADS
  return sigprocmask(SIG_SETMASK, &newset, nullptr);
#endif // NOTHREADS
}

int shrpx_signal_set(sigset_t *set) {
#ifndef NOTHREADS
  int rv;

  rv = pthread_sigmask(SIG_SETMASK, set, nullptr);

  if (rv != 0) {
    errno = rv;
    return -1;
  }

  return 0;
#else  // NOTHREADS
  return sigprocmask(SIG_SETMASK, set, nullptr);
#endif // NOTHREADS
}

namespace {
template <typename Signals>
void signal_set_handler(void (*handler)(int), Signals &&sigs) {
  struct sigaction act {};
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  for (auto sig : sigs) {
    sigaction(sig, &act, nullptr);
  }
}
} // namespace

namespace {
constexpr auto master_proc_ign_signals = std::array<int, 1>{{SIGPIPE}};
} // namespace

namespace {
constexpr auto worker_proc_ign_signals = std::array<int, 4>{
    {REOPEN_LOG_SIGNAL, EXEC_BINARY_SIGNAL, GRACEFUL_SHUTDOWN_SIGNAL, SIGPIPE}};
} // namespace

void shrpx_signal_set_master_proc_ign_handler() {
  signal_set_handler(SIG_IGN, master_proc_ign_signals);
}

void shrpx_signal_unset_master_proc_ign_handler() {
  signal_set_handler(SIG_DFL, master_proc_ign_signals);
}

void shrpx_signal_set_worker_proc_ign_handler() {
  signal_set_handler(SIG_IGN, worker_proc_ign_signals);
}

void shrpx_signal_unset_worker_proc_ign_handler() {
  signal_set_handler(SIG_DFL, worker_proc_ign_signals);
}

} // namespace shrpx
