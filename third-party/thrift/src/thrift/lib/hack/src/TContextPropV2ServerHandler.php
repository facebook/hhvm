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

type ServerInstrumentationParams = shape(
  ?'fn_name' => string,
  ?'fn_args' => mixed,
  ?'fn_result' => mixed,
);

final class TContextPropV2ServerHandler extends TProcessorEventHandler {
  private vec<IContextHandler> $handlers;

  public function __construct(
    private ThriftServer $thriftServer,
    private ServerInstrumentationParams $params,
  ) {
    $this->params = $params;
    $this->handlers = vec[];
    $this->registerHandlers();
  }

  private function registerHandlers(): void {
    // register handlers
    $this->addHandler(new ExperimentIdContextHandler());
  }

  public function addHandler(IContextHandler $handler): void {
    $this->handlers[] = $handler;
  }

  private static function isMutableTFMRSet(
    ThriftFrameworkMetadataOnResponse $mutable_tfmr,
  ): bool {
    return
      $mutable_tfmr != ThriftFrameworkMetadataOnResponse::withDefaultValues();
  }

  <<__Override>>
  public function preWrite(
    mixed $_handler_context,
    string $fn_name,
    mixed $result,
  ): void {
    // keeping this logic in tact from v1 and not moving it to handlers
    // because we write to ARTILLERY_TRACING_HEADERS and not TFMR
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

    $full_params = $this->params;
    $full_params['fn_name'] = $fn_name;
    $full_params['fn_result'] = $result;

    $mutable_tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $immutable_ctx =
      new ImmutableThriftContextPropState(ThriftContextPropState::get());
    // call handlers
    foreach ($this->handlers as $handler) {
      $handler->onOutgoingUpstream($full_params, $mutable_tfmr, $immutable_ctx);
    }

    // return if handlers did not change tfmr
    if (!self::isMutableTFMRSet($mutable_tfmr)) {
      return;
    }

    // encode tfmr and add header
    $buffer = TCompactSerializer::serialize($mutable_tfmr);
    $encoded_response_tfm = Base64::encode($buffer);

    $this->thriftServer->addHTTPHeader(
      HTTPResponseHeader::THRIFT_FRAMEWORK_METADATA_RESPONSE,
      $encoded_response_tfm,
    );
  }

}
