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

/**
 * Helper class to install the right context prop thrift handlers based on
 * JKs.
 */
final class ThriftContextPropHandlers {

  public static function install(
    TClientMultiEventHandler $thrift_client_handler,
    ?TTransportSupportsHeaders $headers_transport,
    ClientInstrumentationParams $instrumentation_params,
  ): void {
    // contextprop v2 handler
    // TODO T204080230: add a CPv2 handler for AM so it does not need its own handler
    $context_prop_state = ThriftContextPropState::get();
    try {
      // Check if AM Traffic AND (AM Tenant OR Async job) AND JK passes with consistent rate using Request ID as the hash.
      if (
        (
          $context_prop_state->getRootProductId() ==
            MCPProductID::L4_ADS_MANAGER ||
          MCPContext::getGlobalOriginIdDirectly__UNSAFE() ==
            MCPProductID::L4_ADS_MANAGER
        ) &&
        (
          RuntimeTenant::isAdsManagerTenant() ||
          Environment::isAsyncEnvironment()
        ) &&
        JustKnobs::evalString(
          'am_health/set_thrift_request_priority:is_am_tenant_or_async',
          $context_prop_state->getRequestId(),
        )
      ) {
        $thrift_client_handler->addHandler(
          'am_thrift_context_prop_update_request_priority',
          AMThriftContextPropUpdateRequestPriority::getInstance(),
        );
      }
    } catch (Exception $e) {
      FBLogger('am_request_priority', 'failed_to_register_handler')
        ->handle(
          $e,
          causes_the('AM Request priority handler')->to('be not set'),
        );

    }
    try {
      if (
        (
          $context_prop_state->getRootProductId() ==
            MCPProductID::L4_MARKETING_API ||
          MCPContext::getGlobalOriginIdDirectly__UNSAFE() ==
            MCPProductID::L4_MARKETING_API
        ) &&
        Environment::isAsyncEnvironment() &&
        JustKnobs::evalString(
          "ads/marketing_api/context_prop:set_thrift_request_priority",
          $context_prop_state->getRequestId(),
        )
      ) {
        $thrift_client_handler->addHandler(
          'mapi_thrift_context_prop_update_request_priority',
          MAPIThriftContextPropUpdateRequestPriority::getInstance(),
        );
      }
    } catch (Exception $e) {
      FBLogger('mapi_request_priority', 'failed_to_register_handler')
        ->handle(
          $e,
          causes_the('MAPI Request priority handler')->to('not be set'),
        );
    }
    if (JustKnobs::evalFast("servicerouter/thrift_context_prop", "enabled")) {
      $client_handler = new TContextPropV2ClientHandler(
        $headers_transport,
        $instrumentation_params,
      );

      if (
        JustKnobs::eval(
          'privacy_infra/xsu:thrift_context_prop_update_universe_handler',
        )
      ) {
        $client_handler->addHandler(new UpdateUniverseContextHandler());
      }
      $client_handler->addHandler(new ExperimentIdContextHandler());
      $client_handler->addHandler(new ProductIdContextHandler());
      if (JustKnobs::eval('artillery/sdk_www:enable_trace_size_propagation')) {
        $client_handler->addHandler(
          TraceSizeEstimationContextHandler::getInstance(),
        );
      }
      //TODO T204080230: add AM handler equivalent

      $thrift_client_handler->addHandler(
        'thrift_context_prop_v2',
        $client_handler,
      );
    }
  }
}
