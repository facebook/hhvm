/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

/**
 * This class interfaces with the fork_intercept library to control if forking
 * is allowed or not. By default it disallows forking and should remain so
 * during normal operation. However, during testing, forking can be allowed for
 * testing purposes.
 */
class fork_intercept {
  static bool intercept;

 public:
  void static set_intercept(bool should_crash) { intercept = should_crash; }

  bool static get_intercept() { return intercept; }
};
