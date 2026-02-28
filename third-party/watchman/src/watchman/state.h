/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/thirdparty/jansson/jansson.h"

void w_state_shutdown();
void w_state_save();
bool w_state_load();

bool w_root_save_state(json_ref& state);
bool w_root_load_state(const json_ref& state);
