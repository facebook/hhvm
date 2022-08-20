/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Function.h>
#include <folly/Memory.h>
#include <glog/logging.h>
#include <memory>
#include <utility>

namespace proxygen {

/**
 * A generic two-way filter. That is, this filter intercepts calls from an
 * object A to some object B on an interface T1. It also intercepts calls from B
 * to A on an interface T2.
 *
 * Subclass GenericFilter templatized on the appropriate interfaces to intercept
 * calls between T1 and T2.
 *
 * T2 must have the ability to tell T1 to call it back using a certain
 * object. Many different callback patterns have a different name for this
 * function, but this filter needs to be informed when the callback object
 * changes. Therefore, when you subclass this object, you must call
 *
 * void setCallbackInternal(T2*);
 *
 * from the appropriate virtual function and not forward that function call
 * along the chain, or the filter will not work and may have errors!
 */
template <typename T1,
          typename T2,
          void (T1::*set_callback)(T2*),
          bool TakeOwnership,
          typename Dp = std::default_delete<T1>>
class GenericFilter
    : public T1
    , public T2 {
 public:
  using Filter = GenericFilter<T1, T2, set_callback, TakeOwnership, Dp>;
  /**
   * @param calls You will intercept calls to T1 interface iff you
   *              pass true for this parameter.
   * @param callbacks You will intercept calls to T2 interface iff you
   *                  pass true for this parameter.
   */
  GenericFilter(bool calls, bool callbacks)
      : kWantsCalls_(calls), kWantsCallbacks_(callbacks) {
  }

  ~GenericFilter() override {
    if (TakeOwnership) {
      callbackSource_ = nullptr;
    }
    // For the last filter in the chain, next_ is null, and call_ points
    // to the concrete implementation object.
    auto next = next_ ? next_ : call_;
    drop();
    if (TakeOwnership && next) {
      Dp()(next);
    }
  }

  /**
   * @param nextFilter the new filter to insert after this filter
   */
  void append(Filter* nextFilter) {
    nextFilter->next_ = next_;
    nextFilter->prev_ = this;
    nextFilter->call_ = call_;
    nextFilter->callback_ = kWantsCallbacks_ ? this : callback_;
    nextFilter->callSource_ = kWantsCalls_ ? this : callSource_;
    nextFilter->callbackSource_ = callbackSource_;
    if (next_) {
      next_->prev_ = nextFilter;
    }

    if (nextFilter->kWantsCalls_) {
      if (kWantsCalls_) {
        call_ = nextFilter;
      } else {
        callSource_->call_ = nextFilter;
      }
      if (next_) {
        next_->callSource_ = nextFilter;
      }
    }
    if (nextFilter->kWantsCallbacks_) {
      // Find the first filter before this one that wants callbacks
      auto cur = this;
      while (cur->prev_ && !cur->kWantsCallbacks_) {
        cur = cur->prev_;
      }
      cur->callbackSource_ = nextFilter;
      // Make sure nextFilter gets callbacks
      ((nextFilter->callbackSource_)->*(set_callback))(nextFilter);
    }
    next_ = nextFilter;
  }

  const bool kWantsCalls_;
  const bool kWantsCallbacks_;

 protected:
  /**
   * Note: you MUST override the set_callback function and call
   * setCallbackInternal() from your derived class.
   */
  void setCallbackInternal(T2* cb) {
    setCallbackInternalImpl(cb, this);
  }

  /**
   * Removes this filter from the chain. For owning chains the caller must
   * manually delete the filter
   */
  void drop() {
    if (prev_) {
      prev_->next_ = next_;
    }
    if (next_) {
      next_->prev_ = prev_;
    }
    // TODO: could call fn gated by std::enable_if
    if (kWantsCalls_ && callSource_) {
      callSource_->call_ = call_;
      if (call_) {
        auto callFilter = dynamic_cast<Filter*>(call_);
        if (callFilter) {
          callFilter->callSource_ = callSource_;
        }
      }
    }
    // TODO: could call fn gated by std::enable_if
    if (kWantsCallbacks_ && callbackSource_) {
      ((callbackSource_)->*(set_callback))(callback_);
      if (callback_) {
        auto callbackFilter = dynamic_cast<Filter*>(callback_);
        if (callbackFilter) {
          callbackFilter->callbackSource_ = callbackSource_;
        }
      }
    }
    call_ = callbackSource_ = nullptr;
    callback_ = nullptr;
    callSource_ = next_ = prev_ = nullptr;
  }

  // Next "call" filter (call_ == next_ iff next_->kWantsCalls_)
  T1* call_{nullptr};
  // Next "callback" filter (callback_ == prev_ iff prev_->kWantsCallbacks_)
  T2* callback_{nullptr};

 private:
  void setCallbackInternalImpl(T2* cb, T2* sourceSet) {
    if (callback_ != cb) {
      callback_ = cb;
      ((callbackSource_)->*(set_callback))(cb ? sourceSet : nullptr);
    }
  }

  // The next filter in the chain (towards T1 implementation)
  Filter* next_{nullptr};
  // The previous filter in the chain (towards T2 implementation)
  Filter* prev_{nullptr};
  // The first filter before this one in the chain that wants calls.
  // Only valid if kWantsCalls_ is true.
  Filter* callSource_{nullptr};
  // Pointer to the first filter or object after this one in the chain
  // that makes callbacks. Only valid if kWantsCallbacks_ is true.
  T1* callbackSource_{nullptr};

  template <class A, class B, class F, void (A::*fn)(B*), bool Own>
  friend class FilterChain;
};

/**
 * This class can be treated the same as a T1*, however internally it contains a
 * chain of filters for T1 and T2. These filters are inserted between the
 * original callback and destination.
 *
 * If a filter does not care about one side of the calls, it will pass through
 * the calls, saving a virtual function call. In the example below Filter1 only
 * wants to intercept callbacks, and Filter2 only wants to intercept calls.
 *
 *
 *      FilterChain        Filter1          Filter2        destination
 *     _____________    _____________    _____________    _____________
 *     |           |    |           |    |           |    |           |
 *     |   call_------------------------>|   call_------->|"call"     | T1
 *     |           |    |           |    |           |    |           |
 *  T2 | callback_ |<-----callback_ |<--------------------|"callback" |
 *     |___________|    |___________|    |___________|    |___________|
 *
 *
 * This class is templatized on the two interfaces, T1, T2, as well as on the
 * pass through filter implementation, FilterType, the special set callback
 * function, and a boolean indicating whether the chain owns the filters (if
 * false, the filters must delete themselves at the correct time). FilterType
 * must have GenericFilter as an ancestor.
 */
template <typename T1,
          typename T2,
          typename FilterType,
          void (T1::*set_callback)(T2*),
          bool TakeOwnership>
class FilterChain : private FilterType {
 public:
  explicit FilterChain(std::unique_ptr<T1> destination)
      : FilterType(false, false) {
    static_assert(TakeOwnership,
                  "unique_ptr constructor only available "
                  "if the chain owns the filters.");
    this->call_ = CHECK_NOTNULL(destination.release());
    this->callback_ = nullptr; // must call setCallback() explicitly
    this->callSource_ = this;
    this->callbackSource_ = this->call_;
    this->chainEnd_ = this->call_;
  }

  explicit FilterChain(T1* destination) : FilterType(false, false) {
    static_assert(!TakeOwnership,
                  "raw pointer constructor only available "
                  "if the chain doesn't own the filters.");
    this->call_ = CHECK_NOTNULL(destination);
    this->callback_ = nullptr; // must call setCallback() explicitly
    this->callSource_ = this;
    this->callbackSource_ = this->call_;
    this->chainEnd_ = this->call_;
  }

  /**
   * Set the callback for this entire filter chain. Setting this to null will
   * uninstall the callback from the concrete object at the end of the chain.
   */
  void setCallback(T2* cb) override {
    this->setCallbackInternalImpl(cb, cb);
  }

  /**
   * Returns the head of the call chain. Do* not* call T1::set_callback() on
   * this member. To change the callback of this entire filter chain, use the
   * separate setCallback() method.
   */
  T1* call() {
    return this->call_;
  }
  const T1* call() const {
    return this->call_;
  }

  /**
   * Returns the concrete implementation at the end of the filter chain.
   */
  T1* getChainEndPtr() {
    return chainEnd_;
  }
  const T1& getChainEnd() const {
    return *chainEnd_;
  }

  using FilterChainType = GenericFilter<T1, T2, set_callback, TakeOwnership>;
  std::unique_ptr<T1> setDestination(std::unique_ptr<T1> destination) {
    static_assert(TakeOwnership,
                  "unique_ptr setDestination only available "
                  "if the chain owns the filters.");
    // find the last filter in the chain, and the last filter that wants calls,
    // callbacks
    FilterChainType* lastFilter = this;
    FilterChainType* lastCall = this;
    FilterChainType* lastCallback = this;
    while (lastFilter->next_) {
      if (lastFilter->kWantsCalls_) {
        lastCall = lastFilter;
      }
      if (lastFilter->kWantsCallbacks_) {
        lastCallback = lastFilter;
      }
      if (lastFilter->call_ == this->chainEnd_) {
        // Search and replace, the last N non-call filters all point to dest
        lastFilter->call_ = destination.get();
      }
      lastFilter = lastFilter->next_;
    }
    if (lastFilter->kWantsCalls_) {
      lastCall = lastFilter;
    }
    if (lastFilter->kWantsCallbacks_) {
      lastCallback = lastFilter;
    }
    lastFilter->call_ = CHECK_NOTNULL(destination.release());
    lastCall->call_ = lastFilter->call_;
    lastCallback->callbackSource_ = lastFilter->call_;
    auto oldChainEnd = this->chainEnd_;
    this->chainEnd_ = lastFilter->call_;

    this->chainEnd_->setCallback(lastCallback);
    return std::unique_ptr<T1>(oldChainEnd);
  }

  /**
   * Adds filters with the given types to the front of the chain.
   */
  template <typename C, typename C2, typename... Types>
  typename std::enable_if<std::is_constructible<C>::value>::type addFilters() {
    // Callback <-> F1 <-> F2 ... <-> F_new <-> Destination
    this->append(new C());
    addFilters<C2, Types...>();
  }

  /**
   * Base case of above function where we add a single filter.
   */
  template <typename C>
  typename std::enable_if<std::is_constructible<C>::value>::type addFilters() {
    this->append(new C());
  }

  /**
   * Adds already constructed filters (inside unique_ptr) to the front of the
   * chain.
   */
  template <typename C, typename... Types>
  void addFilters(std::unique_ptr<C> cur, Types&&... remaining) {
    static_assert(TakeOwnership,
                  "addFilters() can only take "
                  "unique_ptr if the chain owns the filters");
    this->append(cur.release());
    addFilters(std::forward<Types>(remaining)...);
  }

  template <typename C, typename... Types>
  void addFilters(C* cur, Types&&... remaining) {
    static_assert(!TakeOwnership,
                  "addFilters() can only take "
                  "pointers if the chain doesn't own the filters");
    this->append(cur);
    addFilters(std::forward<Types>(remaining)...);
  }

  /**
   * Another way to add filters. This way is similar to 'emplace_front' and
   * returns a reference to itself so you can chain add() calls if you like.
   */
  template <typename C, typename... Args>
  FilterChain<T1, T2, FilterType, set_callback, TakeOwnership>& add(
      Args&&... args) {
    this->append(new C(std::forward<Args>(args)...));
    return *this;
  }

  const T1* operator->() const {
    return call();
  }
  T1* operator->() {
    return call();
  }

  void foreach (folly::FunctionRef<void(FilterChainType*)> fn) {
    auto cur = this->next_;
    while (cur) {
      auto filter = cur;
      cur = cur->next_;
      fn(filter);
    }
  }

 private:
  /**
   * Base case for addFilters() called with no arguments. It doesn't need to be
   * public since trying to add zero filters doesn't make sense from a public
   * API pov.
   */
  void addFilters() {
  }

  T1* chainEnd_;
};

} // namespace proxygen
