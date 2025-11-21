#include <folly/portability/GTest.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/request-fanout-limit.h"
#include "hphp/runtime/base/request-id.h"
#include "hphp/system/systemlib.h"

using namespace testing;

namespace HPHP {

class RequestFanoutLimitTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Default limit for most tests
    limit_ = std::make_unique<RequestFanoutLimit>(3, 100);
  }

  void TearDown() override {
    limit_.reset();
  }

  std::unique_ptr<RequestFanoutLimit> limit_;
};

// Test basic constructor and getLimit
TEST_F(RequestFanoutLimitTest, BasicConstruction) {
  EXPECT_EQ(limit_->getLimit(), 3);
}

// Test basic increment functionality
TEST_F(RequestFanoutLimitTest, BasicIncrement) {
  // RequestId id = RequestId();
  auto id = RequestId::allocate();
  EXPECT_GT(id.id(), 0);
  
  // Initial state
  EXPECT_EQ(limit_->getCurrentCount(id), -1);
  
  // increment
  limit_->increment(id);
  EXPECT_EQ(limit_->getCurrentCount(id), 2);
}

// Test basic decrement functionality
TEST_F(RequestFanoutLimitTest, BasicDecrement) {
  RequestId id = RequestId::allocate();
  
  // Increment to 2
  limit_->increment(id);
  EXPECT_EQ(limit_->getCurrentCount(id), 2);
    
  // First decrement
  limit_->decrement(id);
  EXPECT_EQ(limit_->getCurrentCount(id), 1);

  // Second decrement should bring the count to 0 
  // and have the entry erased, in which case getCurrentCount()
  // will return -1 when entry not found
  limit_->decrement(id);
  EXPECT_EQ(limit_->getCurrentCount(id), -1);
}

TEST_F(RequestFanoutLimitTest, HighConcurrency) {
  auto limit = std::make_shared<RequestFanoutLimit>(10000, 10);
  auto root_req_id = RequestId::allocate();
  const int num_threads = 100000;

  std::vector<std::thread> threads(num_threads);
  
  for (std::thread& t : threads) {
    t = std::thread(
      [limit, root_req_id] {
        hphp_thread_init();
        rds::local::init();
        limit->increment(root_req_id);
        limit->decrement(root_req_id);
        rds::local::fini();
        hphp_thread_exit();        
      }
    );
  }
  for (std::thread& t : threads) {
    t.join();
  }
  EXPECT_EQ(limit->getCurrentCount(root_req_id), 1);
}

TEST_F(RequestFanoutLimitTest, IncrementWithExceededLimit) {
  auto id = RequestId::allocate();
  EXPECT_GT(id.id(), 0);
  
  // Initial state
  EXPECT_EQ(limit_->getCurrentCount(id), -1);
  
  // increment should succeed up to the limit
  limit_->increment(id);
  limit_->increment(id);
  EXPECT_EQ(limit_->getCurrentCount(id), 3);

  // Increment should fail when limit is exceeded
  EXPECT_ANY_THROW(limit_->increment(id));
}

} // namespace HPHP
