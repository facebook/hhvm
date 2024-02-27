/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Exporter.h>

namespace fizz {
template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::writeNewSessionTicket(
    WriteNewSessionTicket w) {
  pendingEvents_.push_back(std::move(w));
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::initiateKeyUpdate(
    KeyUpdateInitiation kui) {
  pendingEvents_.push_back(std::move(kui));
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::appWrite(AppWrite w) {
  pendingEvents_.push_back(std::move(w));
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::earlyAppWrite(
    EarlyAppWrite w) {
  pendingEvents_.push_back(std::move(w));
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::appClose() {
  pendingEvents_.push_back(detail::PendingEvent(AppClose::WAIT));
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::appCloseImmediate() {
  pendingEvents_.push_back(detail::PendingEvent(AppClose::IMMEDIATE));
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::waitForData() {
  waitForData_ = true;
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::newTransportData() {
  waitForData_ = false;
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::moveToErrorState(
    const folly::AsyncSocketException& ex) {
  // If we're already in error state, skip delivering additional error
  // callbacks. This prevents recursion if we are invoked within an earlier
  // error callback.
  if (externalError_) {
    return;
  }

  // We use a separate flag here rather than just moving the state to Error
  // since there may be a currently processing action.
  externalError_ = true;
  while (!pendingEvents_.empty()) {
    auto event = std::move(pendingEvents_.front());
    pendingEvents_.pop_front();
    switch (event.type()) {
      case detail::PendingEvent::Type::AppWrite_E:
        if (event.asAppWrite()->callback) {
          event.asAppWrite()->callback->writeErr(0, ex);
        }
        break;
      case detail::PendingEvent::Type::EarlyAppWrite_E:
        if (event.asEarlyAppWrite()->callback) {
          event.asEarlyAppWrite()->callback->writeErr(0, ex);
        }
        break;
      case detail::PendingEvent::Type::AppClose_E:
      case detail::PendingEvent::Type::WriteNewSessionTicket_E:
      case detail::PendingEvent::Type::KeyUpdateInitiation_E:
        break;
    }
  }
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::pause() {
  paused_ = true;
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::resume() {
  paused_ = false;
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
bool FizzBase<Derived, ActionMoveVisitor, StateMachine>::inErrorState() const {
  return state_.state() == decltype(state_.state())::Error;
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
bool FizzBase<Derived, ActionMoveVisitor, StateMachine>::inTerminalState()
    const {
  return inErrorState() || externalError_ ||
      state_.state() == decltype(state_.state())::Closed;
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
bool FizzBase<Derived, ActionMoveVisitor, StateMachine>::actionProcessing()
    const {
  return actionGuard_.has_value();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::processActions(
    typename StateMachine::CompletedActions& actions) {
  // This extra DestructorGuard is needed due to the gap between clearing
  // actionGuard_ and potentially processing another action.
  folly::DelayedDestruction::DestructorGuard dg(owner_);

  visitActions(actions);

  actionGuard_.reset();
  processPendingEvents();
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::addProcessingActions(
    typename StateMachine::ProcessingActions actions) {
  if (actionGuard_) {
    throw std::runtime_error("actions already processing");
  }

  actionGuard_ = folly::DelayedDestruction::DestructorGuard(owner_);

  static_cast<Derived*>(this)->startActions(std::move(actions));
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
void FizzBase<Derived, ActionMoveVisitor, StateMachine>::
    processPendingEvents() {
  if (inProcessPendingEvents_) {
    return;
  }

  folly::DelayedDestruction::DestructorGuard dg(owner_);
  inProcessPendingEvents_ = true;
  SCOPE_EXIT {
    inProcessPendingEvents_ = false;
  };

  while (!actionGuard_ && !inTerminalState() && !paused_) {
    folly::Optional<typename StateMachine::ProcessingActions> actions;
    actionGuard_ = folly::DelayedDestruction::DestructorGuard(owner_);
    if (!waitForData_) {
      actions.emplace(machine_.processSocketData(
          state_, transportReadBuf_, readAeadOptions_));
    } else if (!pendingEvents_.empty()) {
      auto event = std::move(pendingEvents_.front());
      pendingEvents_.pop_front();
      switch (event.type()) {
        case detail::PendingEvent::Type::WriteNewSessionTicket_E:
          actions.emplace(machine_.processWriteNewSessionTicket(
              state_, std::move(*event.asWriteNewSessionTicket())));
          break;
        case detail::PendingEvent::Type::AppWrite_E:
          actions.emplace(
              machine_.processAppWrite(state_, std::move(*event.asAppWrite())));
          break;
        case detail::PendingEvent::Type::EarlyAppWrite_E:
          actions.emplace(machine_.processEarlyAppWrite(
              state_, std::move(*event.asEarlyAppWrite())));
          break;
        case detail::PendingEvent::Type::AppClose_E:
          if (event.asAppClose()->policy == AppClose::WAIT) {
            actions.emplace(machine_.processAppClose(state_));
          } else {
            actions.emplace(machine_.processAppCloseImmediate(state_));
          }
          break;
        case detail::PendingEvent::Type::KeyUpdateInitiation_E:
          actions.emplace(machine_.processKeyUpdateInitiation(
              state_, std::move(*event.asKeyUpdateInitiation())));
          break;
      }
    } else {
      actionGuard_.reset();
      return;
    }

    static_cast<Derived*>(this)->startActions(std::move(*actions));
  }
}

template <typename Derived, typename ActionMoveVisitor, typename StateMachine>
Buf FizzBase<Derived, ActionMoveVisitor, StateMachine>::
    getExportedKeyingMaterial(
        const Factory& factory,
        folly::StringPiece label,
        Buf context,
        uint16_t length) const {
  if (!state_.cipher() || !state_.exporterMasterSecret()) {
    return nullptr;
  }
  return Exporter::getExportedKeyingMaterial(
      factory,
      *state_.cipher(),
      (*state_.exporterMasterSecret())->coalesce(),
      label,
      std::move(context),
      length);
}
} // namespace fizz
