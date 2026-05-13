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

#include <thrift/lib/python/server/request_context_holder.h>

namespace apache::thrift::python {

Cpp2RequestContextHolder::Cpp2RequestContextHolder(Cpp2RequestContext* c)
    : ctx(c), methodName(c ? c->getMethodName() : std::string{}) {}

Cpp2RequestContext* Cpp2RequestContextHolder::get_ctx() const {
  return ctx.load(std::memory_order_relaxed);
}

bool Cpp2RequestContextHolder::is_valid() const {
  return get_ctx() != nullptr;
}

void Cpp2RequestContextHolder::invalidate() {
  ctx.store(nullptr, std::memory_order_relaxed);
}

void installInvalidator(
    Cpp2RequestContext* ctx,
    const std::shared_ptr<Cpp2RequestContextHolder>& holder) {
  if (ctx == nullptr) {
    return;
  }
  auto* ref = new std::shared_ptr<Cpp2RequestContextHolder>(holder);
  ctx->setRequestData(ref, [](void* p) {
    auto* sp = static_cast<std::shared_ptr<Cpp2RequestContextHolder>*>(p);
    (*sp)->invalidate();
    delete sp;
  });
}

} // namespace apache::thrift::python
