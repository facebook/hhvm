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

namespace apache::thrift {

/**
 * The language binding that issued a client request.
 *
 * Not every binding brings its own channel. thrift-python's OmniClient drives
 * the same RequestChannel, event-handler list and ContextStack that a generated
 * C++ client does, so a TProcessorEventHandler cannot otherwise tell the two
 * apart. Handlers that attribute traffic per language read this off the
 * TConnectionContext they are handed.
 *
 * Per request rather than per handler: one handler instance serves every client
 * in a process, so only the request knows which binding issued it.
 *
 * Only bindings that need to be distinguished appear here. thrift-py3 wraps a
 * generated C++ client and reports `Cpp`.
 */
enum class ClientRuntime {
  Cpp,
  /// thrift-python, via OmniClient.
  Python,
  /// Rust, via srclient / bareclient.
  Rust,
};

} // namespace apache::thrift
