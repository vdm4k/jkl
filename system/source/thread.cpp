#include <system/thread.h>

#ifdef __linux__
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)

#endif

namespace jkl::system {

result thread::set_name(const std::string& name) {
  result res{is_thread_running()};
  if (!res) return res;

  _name = name;
#ifdef __linux__
  int rc = pthread_setname_np(_thread.native_handle(), name.c_str());
  bool ok = rc == 0;
  if (!ok) {
    switch (rc) {
      case ERANGE: {
        return result{
            "the length of the string specified pointed to by name exceeds the "
            "allowed limit"};
        break;
      }
      default: {
        return result{"error code not defined"};
        break;
      }
    }
  }
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)

#endif
  return {};
}

result thread::set_affinity(std::vector<size_t> const& core_ids) {
  result res{is_thread_running()};
  if (!res) return res;

#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  for (auto core_id : core_ids) CPU_SET(core_id, &cpuset);
  int rc = pthread_setaffinity_np(_thread.native_handle(), sizeof(cpu_set_t),
                                  &cpuset);
  bool ok = rc == 0;
  if (!ok) {
    switch (rc) {
      case EFAULT: {
        return result{"a supplied memory address was invalid"};
        break;
      }
      case EINVAL: {
        return result{
            "the affinity bit mask mask contains no processors "
            "that are currently physically on the system and permitted to the "
            "thread"};
        break;
      }
      case ESRCH: {
        return result{"no thread with the id thread could be found"};
        break;
      }
      default: {
        return result{"error code not defined"};
        break;
      }
    }
  }

#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  return result{"unsupported functionality"};
#else
  return result{"unsupported functionality"};
#endif

  return res;
}

expected<std::vector<size_t>> thread::get_affinity() {
  expected<std::vector<size_t>> expect{is_thread_running()};
  if (!expect) return expect;

#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);

  int rc = pthread_getaffinity_np(_thread.native_handle(), sizeof(cpu_set_t),
                                  &cpuset);
  bool ok = rc == 0;
  if (ok) {
    for (size_t core_id = 0; core_id < CPU_SETSIZE; ++core_id) {
      if (CPU_ISSET(core_id, &cpuset)) expect._value.push_back(core_id);
    }
  } else {
    switch (rc) {
      case EFAULT: {
        return result{"a supplied memory address was invalid"};
        break;
      }
      case EINVAL: {
        return result{
            "cpusetsize is smaller than the size of the affinity mask used by "
            "the kernel."};
        break;
      }
      case ESRCH: {
        return result{"no thread with the id thread could be found"};
        break;
      }
      default: {
        return result{"error code not defined"};
        break;
      }
    }
  }

#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  return result{"unsupported functionality"};
#else
  return result{"unsupported functionality"};
#endif

  return expect;
}

std::thread const& thread::get_std_thread() const { return _thread; }

result thread::is_thread_running() const noexcept {
  if (!is_running()) {
    return result{"thread is not in running"};
  }
  return {};
}

bool thread::set_config(thread_config* config) {
  if (config) {
    _thread_config = *config;
    return true;
  }
  return false;
}

bool thread::has_config() const noexcept {
  return _thread_config._statistic || _thread_config._logic_call ||
         _thread_config._sleep;
}

void thread::update_statistic() {
  copy_statistic(&_prev_statistic, &_actual_statistic);
}

void thread::copy_statistic(thread_statistic* to,
                            thread_statistic* from) noexcept {
  bool expect{false};
  while (!_write_stat.compare_exchange_strong(expect, true))
    ;
  memcpy(to, from, sizeof(thread_statistic));
  _write_stat.store(false, std::memory_order_release);
}

thread_statistic thread::get_statistic() noexcept {
  thread_statistic stat;
  copy_statistic(&stat, &_prev_statistic);
  return stat;
}

}  // namespace jkl::system
