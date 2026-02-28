#include <folly/portability/GTest.h>
#include <string>

#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/base/request-id.h"

using namespace testing;

namespace HPHP {

class XboxTransportTest : public ::testing::Test {
protected:
  void SetUp() override {}
};

TEST_F(XboxTransportTest, BasicTransportCreationWithRootReqId) {
  RequestId test_root_id = RequestId::allocate();
  std::string message = "test message";
  std::string reqInitDoc = "";
  
  XboxTransport transport(
      folly::StringPiece(message), 
      test_root_id, 
      folly::StringPiece(reqInitDoc)
  );
  
  // Verify the transport was created successfully
  EXPECT_EQ(transport.getMethod(), Transport::Method::POST);
  EXPECT_EQ(transport.getRootRequestId(), test_root_id);
  EXPECT_FALSE(transport.isDone());
}

TEST_F(XboxTransportTest, TransportCreationWithEmptyRootReqId) {
  RequestId empty_id; 
  std::string message = "test message";
  
  XboxTransport transport(
      folly::StringPiece(message), 
      empty_id, 
      folly::StringPiece("")
  );
  
  // Verify the transport was created successfully with unallocated root_req_id
  EXPECT_EQ(transport.getMethod(), Transport::Method::POST);
  EXPECT_TRUE(transport.getRootRequestId().unallocated());
}

} // namespace HPHP
