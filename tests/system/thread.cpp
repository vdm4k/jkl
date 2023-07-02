// SPDX-License-Identifier: BSD-3-Clause
#include <gtest/gtest.h>
#include <system/thread/thread.h>

namespace bro::system::thread::test {

static std::atomic_bool g_test_plus_called{false};
struct testStruct {
    bool operator()(int left, int right) {
        _left = left;
        _right = right;
        g_test_plus_called = true;
        return true;
    }
    int _left = 0;
    int _right = 0;
};


static int test_plus(int left, int right, int &res) {
  res = left + right;
  g_test_plus_called = true;
  return res;
}

static std::atomic_bool g_minus_called{false};
int test_minus(int left, int right, int &res) {
  res = left - right;
  g_minus_called = true;
  return res;
}

static std::atomic_bool g_pre_fun_called{false};
static void pre_fun(int &res) {
  ASSERT_EQ(res, 0);
  res = 1;
  g_pre_fun_called = true;
}

static std::atomic_bool g_post_fun_called{false};
static void post_fun(int &res) {
  ASSERT_EQ(res, 1);
  res = 0;
  g_post_fun_called = true;
}

TEST(thread, callable) {
  int res{0};
  auto call = callable(test_plus, 1, 2, std::ref(res));
  EXPECT_EQ(call(), 3);
  EXPECT_EQ(res, 3);
}

TEST(thread, no_fun) {
  thread thr;
}

TEST(thread, functor) {
  thread thr;
  g_test_plus_called = false;
  testStruct tSt;
  thr.run(tSt, 1, 2);
  while (!g_test_plus_called)
      ;

  thr.stop();
  EXPECT_FALSE(thr.is_running());
  EXPECT_EQ(tSt._left, 1);
  EXPECT_EQ(tSt._right, 2);
}

TEST(thread, named) {
  thread thr;
  std::string thread_name{"bumbum"};
  EXPECT_FALSE(thr.set_name(thread_name));
  g_test_plus_called = false;
  int res{0};
  thr.run(test_plus, 1, 2, std::ref(res));
  while (!g_test_plus_called)
    ;

  EXPECT_TRUE(thr.set_name(thread_name));
  EXPECT_EQ(thr.get_name(), thread_name);
  thr.stop();
  EXPECT_FALSE(thr.is_running());
  EXPECT_EQ(thr.get_name(), thread_name);
}

TEST(thread, affinity) {
  g_test_plus_called = false;
  int res{0};
  thread thr;
  std::vector<size_t> affinity_cores{0};
  EXPECT_FALSE(thr.set_affinity(affinity_cores));
  thr.run(callable(test_plus, 1, 2, std::ref(res)));
  while (!g_test_plus_called)
    ;
  EXPECT_TRUE(thr.set_affinity(affinity_cores));
  g_test_plus_called = false;
  while (!g_test_plus_called)
    ;
  auto get_affinity_cores = thr.get_affinity();
  EXPECT_TRUE(get_affinity_cores);
  EXPECT_EQ(get_affinity_cores._value, affinity_cores);
}

TEST(thread, main_function_call) {
  {
    int res{0};
    thread thr;
    g_test_plus_called = false;
    thr.run(test_plus, 1, 2, std::ref(res));
    while (!g_test_plus_called)
      ;
    EXPECT_EQ(res, 3);
  }

  {
    int res{0};
    thread thr;
    g_test_plus_called = false;
    thr.run(callable(test_plus, 1, 2, std::ref(res)));
    while (!g_test_plus_called)
      ;
    EXPECT_EQ(res, 3);
  }

  {
    int res{0};
    g_test_plus_called = false;
    thread thr(callable(test_plus, 1, 2, std::ref(res)));
    while (!g_test_plus_called)
      ;
    EXPECT_EQ(res, 3);
  }

  {
    int res{0};
    g_test_plus_called = false;
    thread thr(test_plus, 1, 2, std::ref(res));
    while (!g_test_plus_called)
      ;
    EXPECT_EQ(res, 3);
  }
}

TEST(thread, main_and_business_logic_function_call) {
  {
    int plus_res{0};
    int minus_res{0};
    thread thr;
    g_test_plus_called = false;
    g_minus_called = false;
    thr.run_with_logic(callable(test_plus, 1, 2, std::ref(plus_res)), callable(test_minus, 4, 2, std::ref(minus_res)));
    while (!g_test_plus_called || !g_minus_called)
      ;
    EXPECT_EQ(plus_res, 3);
    EXPECT_EQ(minus_res, 2);
  }

  {
    int plus_res{0};
    int minus_res{0};
    g_test_plus_called = false;
    g_minus_called = false;
    thread thr(callable(test_plus, 1, 2, std::ref(plus_res)), callable(test_minus, 4, 2, std::ref(minus_res)));
    while (!g_test_plus_called || !g_minus_called)
      ;
    EXPECT_EQ(plus_res, 3);
    EXPECT_EQ(minus_res, 2);
  }
}

TEST(thread, main_pre_and_post_called) {
  {
    int pre_post_fun_res{0};
    {
      int plus_res{0};
      thread thr;
      g_test_plus_called = false;
      g_minus_called = false;
      thr.run_with_pre_post(callable(test_plus, 1, 2, std::ref(plus_res)),
                            callable(pre_fun, std::ref(pre_post_fun_res)),
                            callable(post_fun, std::ref(pre_post_fun_res)));
      while (!g_test_plus_called)
        ;
      EXPECT_TRUE(g_pre_fun_called.load(std::memory_order_acquire));
      EXPECT_EQ(plus_res, 3);
      EXPECT_EQ(pre_post_fun_res, 1);
    }
    while (!g_post_fun_called)
      ;
    EXPECT_EQ(pre_post_fun_res, 0);
  }

  {
    int pre_post_fun_res{0};
    {
      int plus_res{0};
      g_test_plus_called = false;
      g_minus_called = false;
      thread thr(callable(test_plus, 1, 2, std::ref(plus_res)),
                 callable(pre_fun, std::ref(pre_post_fun_res)),
                 callable(post_fun, std::ref(pre_post_fun_res)));
      while (!g_test_plus_called)
        ;
      EXPECT_TRUE(g_pre_fun_called.load(std::memory_order_acquire));
      EXPECT_EQ(plus_res, 3);
      EXPECT_EQ(pre_post_fun_res, 1);
    }
    while (!g_post_fun_called)
      ;
    EXPECT_EQ(pre_post_fun_res, 0);
  }
}

TEST(thread, main_and_business_logic_pre_and_post_called) {
  {
    int pre_post_fun_res{0};
    {
      int plus_res{0};
      int minus_res{0};
      thread thr;
      g_test_plus_called = false;
      g_minus_called = false;
      thr.run_with_logic_pre_post(callable(test_plus, 1, 2, std::ref(plus_res)),
                                  callable(test_minus, 4, 2, std::ref(minus_res)),
                                  callable(pre_fun, std::ref(pre_post_fun_res)),
                                  callable(post_fun, std::ref(pre_post_fun_res)));
      while (!g_test_plus_called || !g_minus_called)
        ;
      EXPECT_TRUE(g_pre_fun_called.load(std::memory_order_acquire));
      EXPECT_EQ(plus_res, 3);
      EXPECT_EQ(pre_post_fun_res, 1);
    }
    while (!g_post_fun_called)
      ;
    EXPECT_EQ(pre_post_fun_res, 0);
  }

  {
    int pre_post_fun_res{0};
    {
      int plus_res{0};
      int minus_res{0};
      g_test_plus_called = false;
      g_minus_called = false;
      thread thr(callable(test_plus, 1, 2, std::ref(plus_res)),
                 callable(test_minus, 4, 2, std::ref(minus_res)),
                 callable(pre_fun, std::ref(pre_post_fun_res)),
                 callable(post_fun, std::ref(pre_post_fun_res)));
      while (!g_test_plus_called || !g_minus_called)
        ;
      EXPECT_TRUE(g_pre_fun_called.load(std::memory_order_acquire));
      EXPECT_EQ(plus_res, 3);
      EXPECT_EQ(pre_post_fun_res, 1);
    }
    while (!g_post_fun_called)
      ;
    EXPECT_EQ(pre_post_fun_res, 0);
  }
}


TEST(thread, statistic_for_main_loop) {
  thread thr;
  config conf{};
  conf._flush_statistic = std::chrono::milliseconds(1);
  g_test_plus_called = false;
  testStruct tSt;
  thr.run(callable(tSt, 1, 2), &conf);
  while (!g_test_plus_called)
    ;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  thr.stop();
  EXPECT_FALSE(thr.is_running());
  EXPECT_EQ(tSt._left, 1);
  EXPECT_EQ(tSt._right, 2);
  auto stat = thr.get_statistic();
  EXPECT_EQ(stat._empty_loops, 0);
  EXPECT_NE(stat._loops, 0);
}

TEST(thread, statistic_for_void_main_loop) {
  thread thr;
  config conf{};
  conf._flush_statistic = std::chrono::milliseconds(1);
  g_test_plus_called = false;
  int left{0};
  int right{0};
  thr.run(callable([&left, &right](int l, int r) {left = l; right = r; g_test_plus_called = true; return 0; }, 1, 2), &conf);
  while (!g_test_plus_called)
    ;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  thr.stop();
  EXPECT_FALSE(thr.is_running());
  EXPECT_EQ(left, 1);
  EXPECT_EQ(right, 2);
  auto stat = thr.get_statistic();
  EXPECT_NE(stat._empty_loops, 0);
  EXPECT_NE(stat._loops, 0);
}


} // namespace bro::system::thread::test
