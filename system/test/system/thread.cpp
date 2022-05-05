#include <gtest/gtest.h>
#include <system/thread.h>

namespace jkl::system::test {

std::atomic_bool g_called{false};
int plus(int left, int right, int& res) {
  res = left + right;
  g_called = true;
  return res;
}

TEST(thread, callable) {
  int res{0};
  auto call = callable(plus, 1, 2, std::ref(res));
  EXPECT_EQ(call(), 3);
  EXPECT_EQ(res, 3);
}

TEST(thread, no_fun) { thread thr; }

TEST(thread, named) {
  thread thr;
  std::string thread_name{"bumbum"};
  EXPECT_FALSE(thr.set_name(thread_name));
  g_called = false;
  int res{0};
  thr.run(plus, 1, 2, std::ref(res));
  while (!g_called)
    ;

  EXPECT_TRUE(thr.set_name(thread_name));
  EXPECT_EQ(thr.get_name(), thread_name);
  thr.stop();
  EXPECT_FALSE(thr.is_running());
  EXPECT_EQ(thr.get_name(), thread_name);
}

TEST(thread, affinity) {
  g_called = false;
  int res{0};
  thread thr;
  std::vector<size_t> affinity_cores{0};
  EXPECT_FALSE(thr.set_affinity(affinity_cores));
  thr = thread(callable(plus, 1, 2, std::ref(res)));
  while (!g_called)
    ;
  EXPECT_TRUE(thr.set_affinity(affinity_cores));
  auto get_affinity_cores = thr.get_affinity();
  EXPECT_TRUE(get_affinity_cores);
  EXPECT_EQ(get_affinity_cores._value, affinity_cores);
}

TEST(thread, move) {
  {
    thread thr;
    {
      int res{0};
      thread t_thr;
      g_called = false;
      t_thr.run(plus, 1, 2, std::ref(res));
      while (!g_called)
        ;
      EXPECT_TRUE(t_thr.is_running());
      EXPECT_FALSE(thr.is_running());
      thr = std::move(t_thr);
      EXPECT_EQ(res, 3);
      EXPECT_TRUE(thr.is_running());
      EXPECT_FALSE(t_thr.is_running());
    }
  }

  {
    int res{0};
    thread t_thr;
    g_called = false;
    t_thr.run(plus, 1, 2, std::ref(res));
    while (!g_called)
      ;
    EXPECT_TRUE(t_thr.is_running());
    thread thr(std::move(t_thr));
    EXPECT_EQ(res, 3);
    EXPECT_TRUE(thr.is_running());
    EXPECT_FALSE(t_thr.is_running());
  }
}

TEST(thread, swap) {
  {
    thread thr;
    {
      int res{0};
      thread t_thr;
      g_called = false;
      t_thr.run(plus, 1, 2, std::ref(res));
      while (!g_called)
        ;
      EXPECT_TRUE(t_thr.is_running());
      EXPECT_FALSE(thr.is_running());
      thr.swap(t_thr);
      EXPECT_EQ(res, 3);
      EXPECT_TRUE(thr.is_running());
      EXPECT_FALSE(t_thr.is_running());
    }
  }
}

TEST(thread, one_function) {
  {
    int res{0};
    thread thr;
    g_called = false;
    thr.run(plus, 1, 2, std::ref(res));
    while (!g_called)
      ;
    EXPECT_EQ(res, 3);
  }

  {
    int res{0};
    thread thr;
    g_called = false;
    thr.run(callable(plus, 1, 2, std::ref(res)));
    while (!g_called)
      ;
    EXPECT_EQ(res, 3);
  }

  {
    int res{0};
    g_called = false;
    thread thr(callable(plus, 1, 2, std::ref(res)));
    while (!g_called)
      ;
    EXPECT_EQ(res, 3);
  }
}

}  // namespace jkl::system::test
