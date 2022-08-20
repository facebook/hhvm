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

#include <folly/Executor.h>

namespace folly {

// SequencedExecutor is an executor that provides the following guarantee:
// if add(A) and add(B) were sequenced (i.e. add(B) was called after add(A) call
// had returned to the caller) then execution of A and B will be sequenced
// (i.e. B() can be called only after A() returns) too.
class SequencedExecutor : public virtual Executor {
 public:
  virtual ~SequencedExecutor() override {}
};

} // namespace folly
