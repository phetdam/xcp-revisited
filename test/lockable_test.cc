/**
 * @file lockable_test.cc
 * @author Derek Huang
 * @brief lockable.h unit tests
 * @copyright MIT License
 */

#include "pdxcp/lockable.h"

#include <pthread.h>

#include <cerrno>
#include <cstring>

#include <gtest/gtest.h>

namespace {

/**
 * Base test fixture for lockable tests.
 */
class LockableTest : public ::testing::Test {};

/**
 * Null input check for the boolean lockable getter.
 */
TEST_F(LockableTest, BoolGetNullCheckTest)
{
  PDXCP_LKABLE(pdxcp_bool) lkable;
  pdxcp_bool value;
  EXPECT_EQ(-EINVAL, PDXCP_LKABLE_GET(pdxcp_bool)(nullptr, &value));
  EXPECT_EQ(-EINVAL, PDXCP_LKABLE_GET(pdxcp_bool)(&lkable, nullptr));
}

/**
 * Null input check for the int lockable by-value setter.
 */
TEST_F(LockableTest, IntSetNullCheckTest)
{
  EXPECT_EQ(-EINVAL, PDXCP_LKABLE_SET_V(int)(nullptr, 8888));
}

/**
 * Worker thread payload for the `lkable_bool_task`.
 *
 * @param status Thread status
 * @param lkable `PDXCP_LKABLE(pdxcp_bool)` lockable
 */
struct lkable_bool_payload {
  int status;
  PDXCP_LKABLE(pdxcp_bool) lkable;
};

/**
 * Task run by worker thread to update the state of the bool lockable.
 */
void* lkable_bool_task(void* input)
{
  // get pointer to payload
  auto payload = static_cast<lkable_bool_payload*>(input);
  // set value to true + finish
  if ((payload->status = PDXCP_LKABLE_SET_V(pdxcp_bool)(&payload->lkable, true)))
    return nullptr;
  // done, nothing else to return
  return nullptr;
}

/**
 * Test setting the boolean lockable from another thread.
 */
TEST_F(LockableTest, BoolSetThreadTest)
{
  int status;
  // initialize payload
  lkable_bool_payload payload{0, {false, PTHREAD_MUTEX_INITIALIZER}};
  // start thread with payload
  pthread_t worker_thread;
  status = pthread_create(&worker_thread, NULL, lkable_bool_task, &payload);
  ASSERT_EQ(0, status) << "Thread creation failed: " << std::strerror(-status);
  // join thread, clean up, check status
  status = pthread_join(worker_thread, NULL);
  ASSERT_EQ(0, status) << "Thread cleanup failed: " << std::strerror(-status);
  ASSERT_EQ(0, payload.status) << "Thread error: " << std::strerror(payload.status);
  // get value
  pdxcp_bool value;
  status = PDXCP_LKABLE_GET(pdxcp_bool)(&payload.lkable, &value);
  ASSERT_EQ(0, -status) << "Failed to get value: " << std::strerror(-status);
  // destroy mutex + check value
  status = pthread_mutex_destroy(&payload.lkable.mutex);
  ASSERT_EQ(0, status) << "Failed to destroy mutex: " << std::strerror(status);
  EXPECT_EQ(true, value);
}

/**
 * Worker thread payload for the `lkable_size_t_task`.
 *
 * @param status Thread status
 * @param lkable `PDXCP_LKABLE(size_t)` lockable to set
 * @param new_value Value to set lockable with
 */
struct lkable_size_t_payload {
  int status;
  PDXCP_LKABLE(size_t) lkable;
  std::size_t new_value;
};

/**
 * Task run by worker thread to update the state of the bool lockable.
 */
void* lkable_size_t_task(void* input)
{
  // get pointer to payload
  auto payload = static_cast<lkable_size_t_payload*>(input);
  // set value using new_value + finish. pval used just to prevent ugly wrap
  auto val = payload->new_value;
  if ((payload->status = PDXCP_LKABLE_SET_V(size_t)(&payload->lkable, val)))
    return nullptr;
  // done, nothing else to return
  return nullptr;
}

/**
 * Test setting the `size_t` lockable from another thread.
 */
TEST_F(LockableTest, SizeTypeSetThreadTest)
{
  int status;
  // initialize payload
  lkable_size_t_payload payload{0, {1111, PTHREAD_MUTEX_INITIALIZER}, 8888};
  // start thread with payload
  pthread_t worker_thread;
  status = pthread_create(&worker_thread, NULL, lkable_size_t_task, &payload);
  ASSERT_EQ(0, status) << "Thread creation failed: " << std::strerror(-status);
  // join thread, clean up, check status
  status = pthread_join(worker_thread, NULL);
  ASSERT_EQ(0, status) << "Thread cleanup failed: " << std::strerror(-status);
  ASSERT_EQ(0, payload.status) << "Thread error: " << std::strerror(payload.status);
  // get value
  std::size_t value;
  status = PDXCP_LKABLE_GET(size_t)(&payload.lkable, &value);
  ASSERT_EQ(0, -status) << "Failed to get value: " << std::strerror(-status);
  // destroy mutex + check value
  status = pthread_mutex_destroy(&payload.lkable.mutex);
  ASSERT_EQ(0, status) << "Failed to destroy mutex: " << std::strerror(status);
  EXPECT_EQ(payload.new_value, value);
}

}  // namespace
