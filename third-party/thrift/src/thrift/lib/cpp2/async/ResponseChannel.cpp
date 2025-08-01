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

#include <thrift/lib/cpp2/async/ResponseChannel.h>

const std::string kUnknownErrorCode{"0"};
const std::string kOverloadedErrorCode{"1"};
const std::string kTaskExpiredErrorCode{"2"};
const std::string kProxyTransportExceptionErrorCode{"3"};
const std::string kProxyClientProtocolExceptionErrorCode{"4"};
const std::string kQueueOverloadedErrorCode{"5"};
const std::string kInjectedFailureErrorCode{"14"};
const std::string kServerQueueTimeoutErrorCode{"15"};
const std::string kResponseTooBigErrorCode{"17"};
const std::string kRequestTypeDoesntMatchServiceFunctionType{"21"};
const std::string kAppOverloadedErrorCode{"22"};
const std::string kAppClientErrorCode{"23"};
const std::string kAppServerErrorCode{"24"};
const std::string kMethodUnknownErrorCode{"25"};
const std::string kInteractionIdUnknownErrorCode{"26"};
const std::string kInteractionConstructorErrorErrorCode{"27"};
const std::string kRequestParsingErrorCode{"28"};
const std::string kChecksumMismatchErrorCode{"30"};
const std::string kUnimplementedMethodErrorCode{"31"};
const std::string kTenantQuotaExceededErrorCode{"32"};
const std::string kInteractionLoadsheddedErrorCode{"34"};
const std::string kInteractionLoadsheddedQueueTimeoutErrorCode{"35"};
const std::string kInteractionLoadsheddedOverloadErrorCode{"36"};
const std::string kInteractionLoadsheddedAppOverloadErrorCode{"37"};
const std::string kConnectionClosingErrorCode{"-1"};
