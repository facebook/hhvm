/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/HTTPSourceHolder.h"

namespace proxygen::coro {

/**
 * Represents a chain of HTTPSourceFilter's that can be inserted at the head
 * or the tail.
 */
class FilterChain {
 public:
  FilterChain() = default;
  FilterChain(FilterChain&& goner) = default;
  FilterChain& operator=(FilterChain&& goner) = default;

  void setSource(HTTPSource* source) {
    if (tail_) {
      tail_->setSource(source);
    } else {
      head_.setSource(source);
    }
  }

  HTTPSource* head() {
    // only valid if the chain has a real source
    if (tail_) {
      if (*tail_) { // there's a filter and the last one has a source
        return &head_;
      }
    } else if (head_) { // no filter but head has a source
      return &head_;
    }
    return nullptr;
  }

  void insertFront(HTTPSourceFilter* filter) {
    if (tail_ || head_) {
      filter->setSource(head_.release());
    }
    head_.setSource(filter);
    if (!tail_) {
      tail_ = filter; // First filter, set tail
    }
  }

  void insertEnd(HTTPSourceFilter* filter) {
    if (tail_) {
      // At least one filter, push onto tail
      filter->setSource(tail_->release());
      tail_->setSource(filter);
      tail_ = filter;
    } else {
      insertFront(filter); // no filters, call insertFront
    }
  }

  // release ownership of chain
  HTTPSource* release() {
    return head_.release();
  }

 private:
  HTTPSourceHolder head_;
  HTTPSourceFilter* tail_{nullptr};
};

} // namespace proxygen::coro
