#pragma once
#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "result.h"
#include "thread_config.h"
#include "thread_statistic.h"
#include "time.h"

namespace jkl::system {

/** @defgroup thread
 *  @{
 */

/*!
    \brief callable wrapper - provide lazy call
*/
template <typename Function, typename... Args>
struct callable_helper {
  inline auto operator()() { return std::apply(_fun, _args); }

  Function _fun;
  std::tuple<Args...> _args;
};

/*!
    \brief helper function to create callable
*/
template <typename Callable, typename... Args>
auto callable(Callable&& fun, Args&&... args) {
  static_assert(std::is_invocable<typename std::decay<Callable>::type,
                                  typename std::decay<Args>::type...>::value,
                "jkl::system::thread arguments must be invocable after "
                "conversion to rvalues");
  return callable_helper<Callable, Args...>{std::forward<Callable>(fun),
                                            {std::forward<Args>(args)...}};
};

/**
 * provide thread function with additional logic
 *
 * This class provide next functionality -
 * 1. standart while loop idiom - call function while thread in running state
 * 2. monitoring - check how many loops done and others @see thread_statistic
 * 3. custom configuration @see thread_config
 * 4. OS specific function set name/set affinity
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
  thread(thread&& l) noexcept = delete;

  /**
   * assign operator
   *
   * can't do two similar threads
   */
  thread(thread const& l) noexcept = delete;

  /**
   * construct and run thread (similar as std::thread but with running flag)
   *
   * This is good case whan we need loop functionality and/or monitoring
   * @see thread_config
   *
   * @tparam Callable must be callable function or object
   * @tparam Args must arguments for calling function
   * @param fun ref on main function or callable object
   * @param args arguments for calling main function or callable object
   */
  template <typename Callable, typename... Args>
  thread(Callable&& fun, Args&&... args) {
    run(std::forward<Callable>(fun), std::forward<Args>(args)...);
  }

  /**
   * construct and run thread (similar as std::thread but with running flag)
   *
   * This is good case whan we need loop functionality and/or monitoring
   * @see thread_config
   *
   * @tparam InvokeMain must be callable function or object
   * @param invoke_main ref on main function
   */
  template <typename InvokeMain,
            std::enable_if_t<std::is_invocable_v<InvokeMain>, bool> = true>
  thread(InvokeMain&& invoke_main, thread_config* config = nullptr) {
    run(std::forward<InvokeMain>(invoke_main), config);
  }

  /**
   * construct and run thread
   *
   * Sometimes we need to call additional function for example every n
   * microseconds. Also we can do monitoring for this thread and other things
   * @see thread_config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokeLogic must be callable function or object  ( business logic )
   * @param invoke_main main function to call
   * @param invoke_logic additional function to call ( business logic proceed )
   * @param config thread configuration
   */
  template <typename InvokeMain, typename InvokeLogic,
            std::enable_if_t<std::is_invocable_v<InvokeMain> &&
                                 std::is_invocable_v<InvokeLogic>,
                             bool> = true>
  thread(InvokeMain&& invoke_main, InvokeLogic&& invoke_logic,
         thread_config* config = nullptr) {
    run_with_logic(std::forward<InvokeMain>(invoke_main),
                   std::forward<InvokeLogic>(invoke_logic), config);
  }

  /**
   * construct and run thread
   *
   * Call 2 additional function before and after main loop.
   * This is a good one if we need some initialization and free after thread
   * stop. Also we can do monitoring for this thread and other things
   * @see thread_config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokePre must be callable function or object
   * @tparam InvokePost must be callable function or object
   * @param invoke_main main function to call
   * @param invoke_pre will call this function before main loop
   * @param invoke_post will call this function after main loop
   * @param config thread configuration
   */
  template <typename InvokeMain, typename InvokePre, typename InvokePost,
            std::enable_if_t<std::is_invocable_v<InvokeMain> &&
                                 std::is_invocable_v<InvokePre> &&
                                 std::is_invocable_v<InvokePost>,
                             bool> = true>
  thread(InvokeMain&& invoke_main, InvokePre&& invoke_pre,
         InvokePost&& invoke_post, thread_config* config = nullptr) {
    run_with_pre_post(std::forward<InvokeMain>(invoke_main),
                      std::forward<InvokePre>(invoke_pre),
                      std::forward<InvokePost>(invoke_post), config);
  }

  /**
   * construct and run thread
   *
   * Call 2 additional function before and after main loop also call business
   * logic proceed function. This is a good one if we need some initialization
   * and free after thread stop and proceed some business logic. Also we can do
   * monitoring for this thread and other things
   * @see thread_config
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
  template <
      typename InvokeMain, typename InvokeLogic, typename InvokePre,
      typename InvokePost,
      std::enable_if_t<
          std::is_invocable_v<InvokeMain> && std::is_invocable_v<InvokeLogic> &&
              std::is_invocable_v<InvokePre> && std::is_invocable_v<InvokePost>,
          bool> = true>
  thread(InvokeMain&& invoke_main, InvokeLogic&& invoke_logic,
         InvokePre&& invoke_pre, InvokePost&& invoke_post,
         thread_config* config = nullptr) {
    run_with_logic_pre_post(std::forward<InvokeMain>(invoke_main),
                            std::forward<InvokeLogic>(invoke_logic),
                            std::forward<InvokePre>(invoke_pre),
                            std::forward<InvokePost>(invoke_post), config);
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
  thread& operator=(const thread&) = delete;

  /**
   * move assign operator
   *
   * don't want to do because it will look like pimpl.
   * if we want move we can use unique_ptr as wraper
   */
  thread& operator=(thread&& l) = delete;

  /**
   * is thread in running state or not
   *
   * @return true if it's in running state
   */
  bool is_running() const noexcept {
    return _is_active.load(std::memory_order_acquire);
  }

  /**
   * will stop thread if is was running. wait while thread running
   */
  void stop() {
    set_running(false);
    if (_thread.joinable()) _thread.join();
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
  template <typename Callable, typename... Args>
  result run(Callable&& fun, Args&&... args) {
    static_assert(std::is_invocable<typename std::decay<Callable>::type,
                                    typename std::decay<Args>::type...>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");

    if (is_thread_running()) return result{"thread is in running state"};
    auto fn = callable_helper<Callable, Args...>{std::forward<Callable>(fun),
                                                 {std::forward<Args>(args)...}};
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
   * @see thread_config
   *
   * @tparam InvokeMain must be callable function or object
   * @param invoke_main ref on main function
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template <typename InvokeMain>
  result run(InvokeMain&& invoke_main, thread_config* config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    if (is_thread_running()) return result{"thread is in running state"};

    set_config(config);
    _thread =
        std::thread([invoke_main = std::move(invoke_main), this]() mutable {
          set_running();
          if (has_config()) {
            while (is_running()) {
              invoke_main();
              if (_thread_config._sleep) time::sleep(*_thread_config._sleep);
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
   * @see thread_config
   *
   * @tparam InvokeMain must be callable function or object
   * @tparam InvokeLogic must be callable function or object  ( business logic )
   * @param invoke_main main function to call
   * @param invoke_logic additional function to call ( business logic proceed )
   * @param config thread configuration
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  template <typename InvokeMain, typename InvokeLogic>
  result run_with_logic(InvokeMain&& invoke_main, InvokeLogic&& invoke_logic,
                        thread_config* config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value &&
                      std::is_invocable<InvokeLogic>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    if (is_thread_running()) return result{"thread is in running state"};
    set_config(config);
    _thread =
        std::thread([invoke_main = std::move(invoke_main),
                     invoke_logic = std::move(invoke_logic), this]() mutable {
          set_running();
          if (has_config()) {
            while (is_running()) {
              invoke_main();
              invoke_logic();
              if (_thread_config._sleep) time::sleep(*_thread_config._sleep);
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
   * @see thread_config
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
  template <typename InvokeMain, typename InvokePre, typename InvokePost>
  result run_with_pre_post(InvokeMain&& invoke_main, InvokePre&& invoke_pre,
                           InvokePost&& invoke_post,
                           thread_config* config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value &&
                      std::is_invocable<InvokePre>::value &&
                      std::is_invocable<InvokePost>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    if (is_thread_running()) return result{"thread is in running state"};
    set_config(config);
    _thread =
        std::thread([invoke_main = std::move(invoke_main),
                     invoke_pre = std::move(invoke_pre),
                     invoke_post = std::move(invoke_post), this]() mutable {
          set_running();
          invoke_pre();
          if (has_config()) {
            while (is_running()) {
              invoke_main();
              if (_thread_config._sleep) time::sleep(*_thread_config._sleep);
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
   * @see thread_config
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
  template <typename InvokeMain, typename InvokeLogic, typename InvokePre,
            typename InvokePost,
            std::enable_if_t<std::is_invocable_v<InvokeLogic>, bool> = true>
  result run_with_logic_pre_post(InvokeMain&& invoke_main,
                                 InvokeLogic&& invoke_logic,
                                 InvokePre&& invoke_pre,
                                 InvokePost&& invoke_post,
                                 thread_config* config = nullptr) {
    static_assert(std::is_invocable<InvokeMain>::value &&
                      std::is_invocable<InvokeLogic>::value &&
                      std::is_invocable<InvokePre>::value &&
                      std::is_invocable<InvokePost>::value,
                  "jkl::system::thread arguments must be invocable after "
                  "conversion to rvalues");
    if (is_thread_running()) return result{"thread is in running state"};
    set_config(config);

    _thread =
        std::thread([invoke_main = std::move(invoke_main),
                     invoke_logic = std::move(invoke_logic),
                     invoke_pre = std::move(invoke_pre),
                     invoke_post = std::move(invoke_post), this]() mutable {
          set_running();
          invoke_pre();
          if (has_config()) {
            while (is_running()) {
              invoke_main();
              if (_thread_config._sleep) time::sleep(*_thread_config._sleep);
              invoke_logic();
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
  result set_name(std::string const& name);

  /**
   * get thread name
   *
   * @return thread name
   */
  std::string const& get_name() const noexcept { return _name; }

  /**
   * set affinity to specific cores. (Thread must be in running
   * state)
   *
   * @param core_ids core number
   * @return if operation succeed result will be true. otherwise error
   * desription
   */
  result set_affinity(std::vector<size_t> const& core_ids);

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
  std::thread const& get_std_thread() const;

  /**
   * get current thread configuration
   *
   * @return current configuration
   */
  thread_config const& get_config() const noexcept { return _thread_config; }

  /**
   * get current thread statistic
   *
   * will have actual values only if set needed fields in @see thread_config
   * statistic for prev measurement
   *
   * @return thread_statistic @see thread_statistic
   */
  thread_statistic get_statistic() noexcept;

 private:
  /**
   * set running state to thread.
   *
   * if it's in running state thread will stop. otherwise nothing will happen
   */
  void set_running(bool active = true) noexcept {
    _is_active.store(active, std::memory_order_release);
  }

  /**
   * set configuration
   *
   * will save new configuration if provided not null pointer on config
   *
   * @return true if save new configuration
   */
  bool set_config(thread_config* config);

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
  void update_statistic();

  /**
   * copy statistic
   *
   * using lock to prevent race condition and inconsisten data
   */
  void copy_statistic(thread_statistic* to, thread_statistic* from) noexcept;

  std::atomic_bool _is_active{false};   ///< active flag
  std::string _name;                    ///< name of this thread
  thread_config _thread_config;         ///< current configuration
  std::thread _thread;                  ///< internal thread - std thread
  thread_statistic _actual_statistic;   ///< actual statistic
  thread_statistic _prev_statistic;     ///< previous statistic
  std::atomic_bool _write_stat{false};  ///< flag for write stat
};

/** @} */  // end of thread

}  // namespace jkl::system
