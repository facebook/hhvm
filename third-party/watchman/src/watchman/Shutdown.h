/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

namespace watchman {
class Event;
}

using watchman_event = watchman::Event;

bool w_is_stopping();
void w_request_shutdown();
void w_push_listener_thread_event(std::shared_ptr<watchman_event> event);
