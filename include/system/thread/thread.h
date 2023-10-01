#pragma once
#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <system/result.h>
#include <system/time.h>
#include "config.h"
#include "statistic.h"

namespace bro::system::thread {

/** @defgroup thread thread
 *  @{
 */

/*!
    \brief callable wrapper - provide lazy call
*/
template<typename Function, typename... Args>
struct callable_helper {
  /**
   * execute saved function
   */
  inline auto operator()() {
    static_assert(std::is_invocable<typename std::decay<Function>::type, typename std::decay<Args>::type...>::value,
                  "jkl::system::callable_helper arguments must be invocable after "
                  "conversion to rvalues");
    return std::apply(_fun, _args);
  }

  Function _fun;             ///< function to call
  std::tuple<Args...> _args; ///< args to function
};

/*!
    \brief helper function to create callable
*/
template<typename Callable, typename... Args>
auto callable(Callable &&fun, Args &&...args) {
  static_assert(std::is_invocable<typename std::decay<Callable>::type, typename std::decay<Args>::type...>::value,
                "jkl::system::callable arguments must be invocable after "
                "conversion to rvalues");
  return callable_helper<Callable, Args...>{std::forward<Callable>(fun), {std::forward<Args>(args)...}};
};

/**
 * provide thread function with additional logic
 *
 * This class provide next functionality - \n
 * 1. Run Idiom \n
 * 1.a. Standart while loop idiom - call function while thread in running state
 * \n 1.b. With service function - sama as 1.a but with for proceeding
 * additional logic. \n 1.c. With pre and post main function - sama as 1.a but
 * with pre and post while loop function (for initialization thread specific
 * structures). \n
 * 1.d. With service and pre and post functions - 2b + 2c \n
 * 2. Monitoring - can check how many times we proceed functions and how long it
 * was.
 * 3. OS specific functions - set name thread and set affinity thread
 */
class thread {
public:
  /**
   * default constructor
   */
  thread() = default;

  /**
   * move constructor
   *
   * don't want to do because it will look like pimpl.
   * if we want move we can use unique_ptr as wraper
   */
  thread(thread &&l) noexcept = delete;

  /**
   * assign operator
   *
   * can't do two similar threads
   */
  thread(thread const &l) noexcept = delete;

  /**
   * construct and run thread (similar as std::thread but with running flag)
   *
   * This is good case whan we need loop functionality and/or monitoring
   * @see config
   *
   * @tparam Callable must be callable function or object
   * @tparam Args must arguments for calling function
   * @param fun ref on main function or callable object
   * @param args arguments for calling main function or callable object
   */
  template<typename Callable, typename... Args>
  thread(Callable &&fun, Args &&...args) {
    run(std::forward<Callable>(fun), std::forward<Args>(args)...);
  }

  /**
   * construct and run thread (similar as std::thread but with running flag)
   *
   * This is good case whan we need loop functionality and/or monitoring
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @param config thread configuration
   * @param invoke_main ref on main function
   */
  template<typename InvokeMain, std::enable_if_t<std::is_invocable_v<InvokeMain>, bool> = true>
  thread(InvokeMain &&invoke_main, config *config = nullptr) {
    run(std::forward<InvokeMain>(invoke_main), config);
  }

  /**
   * construct and run thread
   *
   * Sometimes we need to call additional function for example every n
   * microseconds. Also we can do monitoring for this thread and other things
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokeLogic must be callable function or object  ( business logic )
   * @param invoke_main main function to call
   * @param invoke_logic additional function to call ( business logic proceed )
   * @param config thread configuration
   */
  template<typename InvokeMain,
           typename InvokeLogic,
           std::enable_if_t<std::is_invocable_v<InvokeMain> && std::is_invocable_v<InvokeLogic>, bool> = true>
  thread(InvokeMain &&invoke_main, InvokeLogic &&invoke_logic, config const *config = nullptr) {
    run_with_logic(std::forward<InvokeMain>(invoke_main), std::forward<InvokeLogic>(invoke_logic), config);
  }

  /**
   * construct and run thread
   *
   * Call 2 additional function before and after main loop.
   * This is a good one if we need some initialization and free after thread
   * stop. Also we can do monitoring for this thread and other things
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokePre must be callable function or object
   * @tparam InvokePost must be callable function or object
   * @param invoke_main main function to call
   * @param invoke_pre will call this function before main loop
   * @param invoke_post will call this function after main loop
   * @param config thread configuration
   */
  template<
    typename InvokeMain,
    typename InvokePre,
    typename InvokePost,
    std::enable_if_t<std::is_invocable_v<InvokeMain> && std::is_invocable_v<InvokePre> && std::is_invocable_v<InvokePost>,
                     bool> = true>
  thread(InvokeMain &&invoke_main, InvokePre &&invoke_pre, InvokePost &&invoke_post, config const *config = nullptr) {
    run_with_pre_post(std::forward<InvokeMain>(invoke_main),
                      std::forward<InvokePre>(invoke_pre),
                      std::forward<InvokePost>(invoke_post),
                      config);
  }

  /**
   * construct and run thread
   *
   * Call 2 additional function before and after main loop also call business
   * logic proceed function. This is a good one if we need some initialization
   * and free after thread stop and proceed some business logic. Also we can do
   * monitoring for this thread and other things
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokeLogic must be callable function or object  ( business logic )
   * @tparam InvokePre must be callable function or object
   * @tparam InvokePost must be callable function or object
   * @param invoke_main main function to call
   * @param invoke_logic additional function to call ( business logic proceed )
   * @param invoke_pre will call this function before main loop
   * @param invoke_post will call this function after main loop
   * @param config thread configuration
   */
  template<
    typename InvokeMain,
    typename InvokeLogic,
    typename InvokePre,
    typename InvokePost,
    std::enable_if_t<
      std::is_invocable_v<
        InvokeMain> && std::is_invocable_v<InvokeLogic> && std::is_invocable_v<InvokePre> && std::is_invocable_v<InvokePost>,
      bool> = true>
  thread(InvokeMain &&invoke_main,
         InvokeLogic &&invoke_logic,
         InvokePre &&invoke_pre,
         InvokePost &&invoke_post,
         config *config = nullptr) {
    run_with_logic_pre_post(std::forward<InvokeMain>(invoke_main),
                            std::forward<InvokeLogic>(invoke_logic),
                            std::forward<InvokePre>(invoke_pre),
                            std::forward<InvokePost>(invoke_post),
                            config);
  }

  /**
   * destructor
   *
   * will stop thread if it is in running state
   */
  ~thread() { stop(); }

  /**
   * assign operator
   *
   * can't do two similar threads
   */
  thread &operator=(thread const &) = delete;

  /**
   * move assign operator
   *
   * don't want to do because it will look like pimpl.
   * if we want move we can use unique_ptr as wraper
   */
  thread &operator=(thread &&l) = delete;

  /**
   * is thread in running state or not
   *
   * @return true if it's in running state
   */
  bool is_running() const noexcept { return _is_active.load(std::memory_order_acquire); }

  /**
   * will stop thread if is was running. wait while thread running
   */
  void stop() {
    set_running(false);
    if (_thread.joinable())
      _thread.join();
  }

  /**
   * run thread (similar as std::thread but with running flag)
   *
   * @tparam Callable object
   * @tparam Args parameters for callable object
   *
   * @param fun callable object
   * @param args arguments
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template<typename Callable, typename... Args>
  result run(Callable &&fun, Args &&...args) {
    static_assert(std::is_invocable<typename std::decay<Callable>::type, typename std::decay<Args>::type...>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");

    if (is_thread_running())
      return result{"thread is in running state"};
    auto fn = callable_helper<Callable, Args...>{std::forward<Callable>(fun), {std::forward<Args>(args)...}};
    _thread = std::thread([fun = std::move(fn), this]() mutable {
      set_running();
      while (is_running()) {
        fun();
      }
      set_running(false);
    });
    while (!is_running())
      ;
    return {};
  }

  /**
   * run thread (similar as std::thread but with running flag)
   *
   * This is good case whan we need loop functionality and/or monitoring
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @param invoke_main ref on main function
   * @param config thread configuration
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template<typename InvokeMain>
  result run(InvokeMain &&invoke_main, config *config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    time::init_timestamp();
    if (is_thread_running())
      return result{"thread is in running state"};

    set_config(config);
    _thread = std::thread([invoke_main = std::move(invoke_main), this]() mutable {
      set_running();
      if (has_config()) {        
        std::chrono::microseconds start{time::get_timestamp()};
        std::chrono::microseconds next_flush_stat{get_flush_stat(start)};
        auto [call_sleep_on_n_loop, call_sleep_on_time] = get_need_sleep(start);
        uint64_t call_sleep_on_n_empty_loop_in_a_row{0};

        while (is_running()) {
          ++_actual_statistic._loops;
          std::chrono::microseconds start_loop = time::get_timestamp();
          next_flush_stat = flush_statistic(next_flush_stat, start_loop);
          std::chrono::microseconds end_loop = invoke_fun_with_stat(invoke_main, _actual_statistic._max_main_function_time, call_sleep_on_n_empty_loop_in_a_row, _actual_statistic._empty_loops);
          _actual_statistic._busy_time += end_loop - start_loop;
          need_to_sleep(call_sleep_on_n_loop, call_sleep_on_time, call_sleep_on_n_empty_loop_in_a_row, end_loop);
        }
      } else {
        while (is_running()) {
          invoke_main();
        }
      }
      set_running(false);
    });
    while (!is_running())
      ;
    return {};
  }

  /**
   * run thread
   *
   * Sometimes we need to call additional function for example every n
   * microseconds. Also we can do monitoring for this thread and other things
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokeLogic must be callable function or object  ( business logic )
   * @param invoke_main main function to call
   * @param invoke_logic additional function to call ( business logic proceed )
   * @param config thread configuration
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template<typename InvokeMain, typename InvokeLogic>
  result run_with_logic(InvokeMain &&invoke_main, InvokeLogic &&invoke_logic, config const *config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value && std::is_invocable<InvokeLogic>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    time::init_timestamp();
    if (is_thread_running())
      return result{"thread is in running state"};
    set_config(config);
    _thread = std::thread(
      [invoke_main = std::move(invoke_main), invoke_logic = std::move(invoke_logic), this]() mutable {
        set_running();
        if (has_config()) {
          std::chrono::microseconds start{time::get_timestamp()};
          std::chrono::microseconds next_flush_stat{get_flush_stat(start)};
          auto [call_sleep_on_n_loop, call_sleep_on_time] = get_need_sleep(start);
          auto[call_logic_on_n_loop, call_logic_on_time] = get_need_call_logic(start);
          uint64_t call_sleep_on_n_empty_loop_in_a_row{0};

          while (is_running()) {
            std::chrono::microseconds start_loop = time::get_timestamp();
            ++_actual_statistic._loops;
            next_flush_stat = flush_statistic(next_flush_stat, start_loop);
            invoke_fun_with_stat(invoke_main, _actual_statistic._max_main_function_time, call_sleep_on_n_empty_loop_in_a_row, _actual_statistic._empty_loops);
            std::chrono::microseconds end_loop = invoke_logic_stat(invoke_logic, call_logic_on_n_loop, call_logic_on_time, start_loop);
            _actual_statistic._busy_time += end_loop - start_loop;
            need_to_sleep(call_sleep_on_n_loop, call_sleep_on_time, call_sleep_on_n_empty_loop_in_a_row, end_loop);
          }
        } else {
          while (is_running()) {
            invoke_main();
            invoke_logic();
          }
        }
        set_running(false);
      });
    while (!is_running())
      ;
    return {};
  }

  /**
   * construct and run thread
   *
   * Call 2 additional function before and after main loop.
   * This is a good one if we need some initialization and free after thread
   * stop. Also we can do monitoring for this thread and other things
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokePre must be callable function or object
   * @tparam InvokePost must be callable function or object
   * @param invoke_main main function to call
   * @param invoke_pre will call this function before main loop
   * @param invoke_post will call this function after main loop
   * @param config thread configuration
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template<typename InvokeMain, typename InvokePre, typename InvokePost>
  result run_with_pre_post(InvokeMain &&invoke_main,
                           InvokePre &&invoke_pre,
                           InvokePost &&invoke_post,
                           config const *config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value && std::is_invocable<InvokePre>::value
                    && std::is_invocable<InvokePost>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    time::init_timestamp();
    if (is_thread_running())
      return result{"thread is in running state"};
    set_config(config);
    _thread = std::thread([invoke_main = std::move(invoke_main),
                           invoke_pre = std::move(invoke_pre),
                           invoke_post = std::move(invoke_post),
                           this]() mutable {
      set_running();
      invoke_pre();
      if (has_config()) {
          std::chrono::microseconds start{time::get_timestamp()};
          std::chrono::microseconds next_flush_stat{get_flush_stat(start)};
          auto [call_sleep_on_n_loop, call_sleep_on_time] = get_need_sleep(start);
          uint64_t call_sleep_on_n_empty_loop_in_a_row{0};

        while (is_running()) {
          ++_actual_statistic._loops;
          std::chrono::microseconds start_loop = time::get_timestamp();
          next_flush_stat = flush_statistic(next_flush_stat, start_loop);
          std::chrono::microseconds end_loop = invoke_fun_with_stat(invoke_main, _actual_statistic._max_main_function_time, call_sleep_on_n_empty_loop_in_a_row, _actual_statistic._empty_loops);
          _actual_statistic._busy_time += end_loop - start_loop;
          need_to_sleep(call_sleep_on_n_loop, call_sleep_on_time, call_sleep_on_n_empty_loop_in_a_row, end_loop);
        }
      } else {
        while (is_running()) {
          invoke_main();
        }
      }
      invoke_post();
      set_running(false);
    });
    while (!is_running())
      ;
    return {};
  }

  /**
   * run thread
   *
   * Call 2 additional function before and after main loop also call business
   * logic proceed function. This is a good one if we need some initialization
   * and free after thread stop and proceed some business logic. Also we can do
   * monitoring for this thread and other things
   * @see config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokeLogic must be callable function or object  ( business logic )
   * @tparam InvokePre must be callable function or object
   * @tparam InvokePost must be callable function or object
   * @param invoke_main main function to call
   * @param invoke_logic additional function to call ( business logic proceed )
   * @param invoke_pre will call this function before main loop
   * @param invoke_post will call this function after main loop
   * @param config thread configuration
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template<typename InvokeMain,
           typename InvokeLogic,
           typename InvokePre,
           typename InvokePost,
           std::enable_if_t<std::is_invocable_v<InvokeLogic>, bool> = true>
  result run_with_logic_pre_post(InvokeMain &&invoke_main,
                                 InvokeLogic &&invoke_logic,
                                 InvokePre &&invoke_pre,
                                 InvokePost &&invoke_post,
                                 config const *config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value && std::is_invocable<InvokeLogic>::value
                    && std::is_invocable<InvokePre>::value && std::is_invocable<InvokePost>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    time::init_timestamp();
    if (is_thread_running())
      return result{"thread is in running state"};
    set_config(config);

    _thread = std::thread([invoke_main = std::move(invoke_main),
                           invoke_logic = std::move(invoke_logic),
                           invoke_pre = std::move(invoke_pre),
                           invoke_post = std::move(invoke_post),
                           this]() mutable {
      set_running();
      invoke_pre();
      if (has_config()) {
          std::chrono::microseconds start{time::get_timestamp()};
          std::chrono::microseconds next_flush_stat{get_flush_stat(start)};
          auto [call_sleep_on_n_loop, call_sleep_on_time] = get_need_sleep(start);
          auto[call_logic_on_n_loop, call_logic_on_time] = get_need_call_logic(start);
          uint64_t call_sleep_on_n_empty_loop_in_a_row{0};

        while (is_running()) {
          std::chrono::microseconds start_loop = time::get_timestamp();
          ++_actual_statistic._loops;
          next_flush_stat = flush_statistic(next_flush_stat, start_loop);
          invoke_fun_with_stat(invoke_main, _actual_statistic._max_main_function_time, call_sleep_on_n_empty_loop_in_a_row, _actual_statistic._empty_loops);
          std::chrono::microseconds end_loop = invoke_logic_stat(invoke_logic, call_logic_on_n_loop, call_logic_on_time, start_loop);
          _actual_statistic._busy_time += end_loop - start_loop;
          need_to_sleep(call_sleep_on_n_loop, call_sleep_on_time, call_sleep_on_n_empty_loop_in_a_row, end_loop);
        }
      } else {
        while (is_running()) {
          invoke_main();
          invoke_logic();
        }
      }
      invoke_post();
      set_running(false);
    });
    while (!is_running())
      ;
    return {};
  }

  /**
   * set thread name (Thread must be in running state)
   *
   * Save thread name and try to set this name in OS. If for some reasons OS
   * can's set name to this thread we will return error but name always saving
   * in class variable => after set_name get_name always return setted value.
   *
   * @param name for this thread
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  result set_name(std::string const &name);

  /**
   * get thread name
   *
   * @return thread name
   */
  std::string const &get_name() const noexcept { return _name; }

  /**
   * set affinity to specific cores. (Thread must be in running
   * state)
   *
   * @param core_ids core number
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  result set_affinity(std::vector<size_t> const &core_ids);

  /**
   * get affinity for thread. Thread must be in running
   * state
   *
   * @return if success status will be true and value will be filled.
   */
  expected<std::vector<size_t>> get_affinity();

  /**
   * get std thread ( const env )
   *
   * @return std thread
   */
  std::thread const &get_std_thread() const;

  /**
   * get current thread configuration
   *
   * @return current configuration
   */
  config const &get_config() const noexcept { return _config; }

  /**
   * get current thread statistic
   *
   * will have actual values only if set needed fields in @see config
   * statistic for prev measurement
   *
   * @return statistic @see statistic
   */
  statistic get_statistic() noexcept;

private:
  /**
   * invoke main function and save statistic about how long execusion was
   *
   * @tparam InvokeFun must be callable function or object
   * @param fun main function to call
   * @param prev_max_time previous maximum
   * @param empty_loops_in_a_row empty loops in a row
   * @param empty_loops empty loops total
   * @return timestamp after function was execute
   */
  template<typename InvokeFun>
  std::chrono::microseconds invoke_fun_with_stat(InvokeFun &fun, std::chrono::microseconds &prev_max_time, uint64_t &empty_loops_in_a_row, uint64_t &empty_loops) {
    std::chrono::microseconds const fun_start_tsc = time::get_timestamp();
    if constexpr(std::is_integral_v<decltype(fun())>) {
      if(fun()) {
          empty_loops_in_a_row = 0;
      } else {
          empty_loops++;
          empty_loops_in_a_row++;
      }
    } else {
      fun();
    }

    std::chrono::microseconds const fun_end_tsc = time::get_timestamp();
    prev_max_time = std::max(prev_max_time, (fun_end_tsc - fun_start_tsc));
    return fun_end_tsc;
  }

  /**
   * invoke logic function
   *
   * invoke immediately if not set delay (call_logic_on_n_loop,
   * call_logic_fun) @see config
   *
   * @tparam InvokeLogic must be callable function or object
   * @param invoke_logic logic function to call
   * @param call_logic_on_n_loop call delay in cycles
   * @param call_logic_fun call delay in time
   * @param cur_tsc current tsc
   * @return timestamp after function was execute
   */
  template<typename InvokeLogic>
  std::chrono::microseconds invoke_logic_stat(InvokeLogic &invoke_logic,
                             uint64_t &call_logic_on_n_loop,
                             std::chrono::microseconds &call_logic_on_time,
                             std::chrono::microseconds cur_tsc) {

    // this two only like placeholders
    uint64_t empty_loops_in_a_row{0};
    uint64_t empty_loops{0};
    // do we need check prerequisites
    if (call_logic_on_n_loop || call_logic_on_time.count()) {

      bool need_to_call{false}; // call only if need to call
      if (call_logic_on_time.count()) {
        need_to_call = call_logic_on_time <= cur_tsc;
        if (need_to_call)
            call_logic_on_time += *_config._call_sleep;
      } else {
        need_to_call = call_logic_on_n_loop <= _actual_statistic._loops;
        if (need_to_call)
            call_logic_on_n_loop += *_config._call_logic_on_n_loop;
      }

      if (need_to_call)
        cur_tsc = invoke_fun_with_stat(invoke_logic, _actual_statistic._max_logic_function_time, empty_loops_in_a_row, empty_loops);
    } else {
      cur_tsc = invoke_fun_with_stat(invoke_logic, _actual_statistic._max_logic_function_time, empty_loops_in_a_row, empty_loops);
    }
    return cur_tsc;
  }

  /**
   * flush statistic
   *
   * will flush statistic only if flush_stat not null and less than cur_tsc
   * @see config
   *
   * @param flush_stat logic function to call
   * @param cur_tsc current tsc
   * @return timestamp on next flush
   */
  std::chrono::microseconds flush_statistic(std::chrono::microseconds flush_stat, std::chrono::microseconds cur_time) noexcept {
    if (flush_stat.count() && flush_stat <= cur_time) {
      flush_statistic();
      flush_stat += *_config._flush_statistic;
    }
    return flush_stat;
  }

  /**
   * sleep execution thread
   *
   * sleep thread only if set needed parameter in config
   * @see config
   *
   * @param call_sleep_on_n_loop delay to call sleep in cycles
   * @param call_sleep delay to call sleep in time
   * @param empty_loop_count how many empty cycles did
   * @param cur_tsc current tsc
   */
  void need_to_sleep(uint64_t &call_sleep_on_n_loop, std::chrono::microseconds &call_sleep, uint64_t &empty_loops_in_a_row, std::chrono::microseconds cur_tsc) const noexcept {
    if (!_config._sleep)
      return;
    bool need_to_sleep{false};
    if (call_sleep_on_n_loop || call_sleep.count() || _config._call_sleep_on_n_empty_loop_in_a_row) {
      bool need_sleep_count{false};
      bool need_on_n_empty_loop_in_a_row{false};
      bool need_sleep_on_n_loop{false};
      if (call_sleep.count()) {
        need_sleep_count = call_sleep <= cur_tsc;
        if (need_sleep_count)
          call_sleep += *_config._call_sleep;
      }
      if(_config._call_sleep_on_n_empty_loop_in_a_row) {
        need_on_n_empty_loop_in_a_row = empty_loops_in_a_row >= _config._call_sleep_on_n_empty_loop_in_a_row;
        if(need_on_n_empty_loop_in_a_row)
          empty_loops_in_a_row = 0;
      }
      if(call_sleep_on_n_loop) {
        need_sleep_on_n_loop = call_sleep_on_n_loop <= _actual_statistic._loops;
        if (need_sleep_on_n_loop)
          call_sleep_on_n_loop += *_config._call_sleep_on_n_loop;
      }
      need_to_sleep = need_sleep_count || need_on_n_empty_loop_in_a_row || need_sleep_on_n_loop;
    } else {
      need_to_sleep = true;
    }
    if (need_to_sleep)
      time::sleep(*_config._sleep);
   }

  /**
   * set running state to thread.
   *
   * if it's in running state thread will stop. otherwise nothing will happen
   */
  void set_running(bool active = true) noexcept { _is_active.store(active, std::memory_order_release); }

  /**
   * set configuration
   *
   * will save new configuration if provided not null pointer on config
   *
   * @return true if save new configuration
   */
  bool set_config(config const *config);

  /**
   * check if configuration has important values ( ex. thread sleep )
   */
  bool has_config() const noexcept;

  /**
   * get status is thread running or not
   *
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  result is_thread_running() const noexcept;

  /**
   * update statistic by working thread
   */
  void flush_statistic();

  /**
   * copy statistic
   *
   * using lock to prevent race condition and inconsisten data
   */
  void copy_statistic(statistic *to, statistic *from) noexcept;

  /**
   * fill sleep parameters
   *
   * @param call_sleep_on_n_loop call sleep on n loop
   * @param call_sleep call sleep in time
   * @param start_tsc current tsc
   */
  std::pair<uint64_t, std::chrono::microseconds> get_need_sleep(std::chrono::microseconds const start) const noexcept;

  /**
   * fill call logic delay parameters
   *
   * @param call_logic_on_n_loop call logic on n loop
   * @param call_logic_fun call logic fun in time
   * @param start_tsc current tsc
   */
  std::pair<uint64_t, std::chrono::microseconds> get_need_call_logic(std::chrono::microseconds const start) const noexcept;

  /**
   * flush statistic
   *
   * @param start_tsc current tsc
   * @return flush statistic timestamp
   */
  std::chrono::microseconds get_flush_stat(std::chrono::microseconds start_tsc);

  std::atomic_bool _is_active{false};  ///< active flag
  std::string _name;                   ///< name of this thread
  config _config;                      ///< current configuration
  std::thread _thread;                 ///< internal thread - std thread
  statistic _actual_statistic;         ///< actual statistic
  statistic _prev_statistic;           ///< previous statistic
  std::atomic_bool _write_stat{false}; ///< flag for write statistic
};

/** @} */ // end of thread

} // namespace bro::system::thread
