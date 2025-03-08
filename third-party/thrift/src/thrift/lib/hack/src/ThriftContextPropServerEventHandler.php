<?hh
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
 *
 */

final class ThriftContextPropServerEventHandler extends TProcessorEventHandler {

  public function __construct(private ThriftServer $thriftServer)[] {}

  // Called before the handler method's $results are written
  <<__Override>>
  public function preWrite(
    mixed $_handler_context,
    string $_fn_name,
    mixed $_result,
  ): void {
    // Each TFM (and TFMOnResponse) is comprised of components defined in
    // fbcode/common/thrift/cpp/server/thrift_framework_metadata/ContextPropComponent.h

    if (
      JustKnobs::eval('artillery/sdk_www:response_header_propagation_rollout')
    ) {
      $context_manager = ContextManager::get();
      if ($context_manager->getCoreContext() !== null) {
        $ctx_prop_headers = $context_manager->processOutgoingResponse(shape());

        if (
          C\contains_key(
            $ctx_prop_headers,
            HTTPResponseHeader::ARTILLERY_TRACING_HEADERS,
          )
        ) {
          $this->thriftServer->addHTTPHeader(
            HTTPResponseHeader::ARTILLERY_TRACING_HEADERS,
            $ctx_prop_headers[HTTPResponseHeader::ARTILLERY_TRACING_HEADERS],
          );
        }
      }
    }

    // TFMOnResponse currently only contains the experiment IDs component,
    // so the TFM response header will only be sent back to clients
    // when the request TFM contains experiment IDs.
    $encoded_response_tfm = ThriftContextPropHandler::makeResponseV();
    if ($encoded_response_tfm is nonnull) {
      $this->thriftServer->addHTTPHeader(
        HTTPResponseHeader::THRIFT_FRAMEWORK_METADATA_RESPONSE,
        $encoded_response_tfm,
      );
    }
  }
}
