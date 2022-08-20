/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/session/HTTPTransactionEgressSM.h>
#include <proxygen/lib/http/session/HTTPTransactionIngressSM.h>

using namespace proxygen;

class EgressStateMachineFixture : public ::testing::Test {
 public:
  EgressStateMachineFixture()
      : instance_(HTTPTransactionEgressSM::getNewInstance()) {
  }

  void follow(HTTPTransactionEgressSM::Event e) {
    EXPECT_TRUE(HTTPTransactionEgressSM::transit(instance_, e));
  }

  void fail(HTTPTransactionEgressSM::Event e) {
    EXPECT_FALSE(HTTPTransactionEgressSM::transit(instance_, e));
  }

 private:
  HTTPTransactionEgressSM::State instance_;
};

class IngressStateMachineFixture : public ::testing::Test {
 public:
  IngressStateMachineFixture()
      : instance_(HTTPTransactionIngressSM::getNewInstance()) {
  }

  void follow(HTTPTransactionIngressSM::Event e) {
    EXPECT_TRUE(HTTPTransactionIngressSM::transit(instance_, e));
  }

  void fail(HTTPTransactionIngressSM::Event e) {
    EXPECT_FALSE(HTTPTransactionIngressSM::transit(instance_, e));
  }

 private:
  HTTPTransactionIngressSM::State instance_;
};

// Egress tests

TEST_F(EgressStateMachineFixture, BadEgressTransitions1) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendChunkHeader);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  fail(HTTPTransactionEgressSM::Event::sendEOM);
}

TEST_F(EgressStateMachineFixture, BadEgressTransitions2) {
  fail(HTTPTransactionEgressSM::Event::sendBody);
}

TEST_F(EgressStateMachineFixture, BadEgressTransitions3) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
  fail(HTTPTransactionEgressSM::Event::sendEOM);
}

TEST_F(EgressStateMachineFixture, BadEgressTransitions4) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendChunkHeader);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendChunkTerminator);
  follow(HTTPTransactionEgressSM::Event::sendChunkHeader);
  fail(HTTPTransactionEgressSM::Event::sendChunkTerminator);
}

TEST_F(EgressStateMachineFixture, EgressChunkedTransitions) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);

  follow(HTTPTransactionEgressSM::Event::sendChunkHeader);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendChunkTerminator);

  follow(HTTPTransactionEgressSM::Event::sendChunkHeader);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendChunkTerminator);

  follow(HTTPTransactionEgressSM::Event::sendChunkHeader);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendChunkTerminator);

  follow(HTTPTransactionEgressSM::Event::sendTrailers);

  follow(HTTPTransactionEgressSM::Event::sendEOM);
  follow(HTTPTransactionEgressSM::Event::eomFlushed);
}

TEST_F(EgressStateMachineFixture, NormalEgressTransitions) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
}

TEST_F(EgressStateMachineFixture, NormalEgressTransitionsWithTrailers) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  follow(HTTPTransactionEgressSM::Event::sendTrailers);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
  follow(HTTPTransactionEgressSM::Event::eomFlushed);
}

TEST_F(EgressStateMachineFixture, WeirdEgressTransitions) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendTrailers);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
  follow(HTTPTransactionEgressSM::Event::eomFlushed);
}

TEST_F(EgressStateMachineFixture, NormalDatagramTransitions) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
  follow(HTTPTransactionEgressSM::Event::eomFlushed);
}

TEST_F(EgressStateMachineFixture, NormalDatagramTransitionsWithTrailers) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendTrailers);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
  follow(HTTPTransactionEgressSM::Event::eomFlushed);
}

TEST_F(EgressStateMachineFixture, BadDatagramTransitionBeforeHeaders) {
  fail(HTTPTransactionEgressSM::Event::sendDatagram);
}

TEST_F(EgressStateMachineFixture, BadDatagramTransitionAfterBody) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendBody);
  fail(HTTPTransactionEgressSM::Event::sendDatagram);
}

TEST_F(EgressStateMachineFixture, BadDatagramTransitionAfterTrailers) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendTrailers);
  fail(HTTPTransactionEgressSM::Event::sendDatagram);
}

TEST_F(EgressStateMachineFixture, BadDatagramTransitionAfterEOM) {
  follow(HTTPTransactionEgressSM::Event::sendHeaders);
  follow(HTTPTransactionEgressSM::Event::sendDatagram);
  follow(HTTPTransactionEgressSM::Event::sendEOM);
  follow(HTTPTransactionEgressSM::Event::eomFlushed);
  fail(HTTPTransactionEgressSM::Event::sendDatagram);
}

// Ingress tests

TEST_F(IngressStateMachineFixture, BadIngressTransitions1) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onChunkHeader);
  follow(HTTPTransactionIngressSM::Event::onBody);
  fail(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, BadIngressTransitions2) {
  fail(HTTPTransactionIngressSM::Event::onBody);
}

TEST_F(IngressStateMachineFixture, BadIngressTransitions3) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onEOM);
  fail(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, BadIngressTransitions4) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onChunkHeader);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onChunkComplete);
  follow(HTTPTransactionIngressSM::Event::onChunkHeader);
  fail(HTTPTransactionIngressSM::Event::onChunkComplete);
}

TEST_F(IngressStateMachineFixture, IngressChunkedTransitions) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);

  follow(HTTPTransactionIngressSM::Event::onChunkHeader);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onChunkComplete);

  follow(HTTPTransactionIngressSM::Event::onChunkHeader);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onChunkComplete);

  follow(HTTPTransactionIngressSM::Event::onChunkHeader);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onChunkComplete);

  follow(HTTPTransactionIngressSM::Event::onTrailers);
  follow(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, NormalIngressTransitions) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onBody);
  follow(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, WeirdIngressTransitions) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onTrailers);
  follow(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, NormalDatagramTransitions) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, NormalDatagramTransitionsWithTrailers) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onTrailers);
  follow(HTTPTransactionIngressSM::Event::onEOM);
}

TEST_F(IngressStateMachineFixture, BadDatagramTransitionBeforeHeaders) {
  fail(HTTPTransactionIngressSM::Event::onDatagram);
}

TEST_F(IngressStateMachineFixture, BadDatagramTransitionAfterBody) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onBody);
  fail(HTTPTransactionIngressSM::Event::onDatagram);
}

TEST_F(IngressStateMachineFixture, BadDatagramTransitionAfterTrailers) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onTrailers);
  fail(HTTPTransactionIngressSM::Event::onDatagram);
}

TEST_F(IngressStateMachineFixture, BadDatagramTransitionAfterEOM) {
  follow(HTTPTransactionIngressSM::Event::onHeaders);
  follow(HTTPTransactionIngressSM::Event::onDatagram);
  follow(HTTPTransactionIngressSM::Event::onEOM);
  fail(HTTPTransactionIngressSM::Event::onDatagram);
}
