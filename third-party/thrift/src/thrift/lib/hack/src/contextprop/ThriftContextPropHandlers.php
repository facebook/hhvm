<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
    if (
      JustKnobs::evalFast('artillery/sdk_www', 'thrift_contextprop_v2_handler')
    ) {
      // TODO T204080230: add a CPv2 handler for AM so it does not need its own handler
      try {
        $context_prop_state = ThriftContextPropState::get();
        // Check if AM Traffic AND (AM Tenant OR Async job) AND JK passes with consistent rate using Request ID as the hash.
        if (
          $context_prop_state->getOriginId() == MCPProductID::L4_ADS_MANAGER &&
          (
            Environment::getServiceId() == 'web_ads_manager/hhvm' ||
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
            Causes::the('AM Request priority handler')->to('be not set'),
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
        //TODO T204080230: add AM handler equivalent

        $thrift_client_handler->addHandler(
          'thrift_context_prop_v2',
          $client_handler,
        );
      }
    } else {
      //contextprop v1 handler
      // NOTE: Always register these handlers together in this order, so the
      // context prop Universe is updated before it's serialized.
      if (
        JustKnobs::eval(
          'privacy_infra/xsu:thrift_context_prop_update_universe_handler',
        )
      ) {
        $service_name = Shapes::idx($instrumentation_params, 'service_name');
        $thrift_class = Shapes::idx($instrumentation_params, 'thrift_class');
        $client = Shapes::idx($instrumentation_params, 'client');
        if (
          $service_name is nonnull &&
          $thrift_class is nonnull &&
          $client is nonnull
        ) {
          $thrift_client_handler->addHandler(
            'thrift_context_prop_update_universe',
            new ThriftContextPropUpdateUniverseHandler(
              $service_name,
              HH_FIXME::tryClassToClassname($thrift_class),
              $client,
            ),
          );
        }

      }

      try {
        $context_prop_state = ThriftContextPropState::get();
        // Check if AM Traffic AND (AM Tenant OR Async job) AND JK passes with consistent rate using Request ID as the hash.
        if (
          $context_prop_state->getOriginId() == MCPProductID::L4_ADS_MANAGER &&
          (
            Environment::getServiceId() == 'web_ads_manager/hhvm' ||
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
            Causes::the('AM Request priority handler')->to('be not set'),
          );

      }
      // contextprop handlers
      if (JustKnobs::evalFast("servicerouter/thrift_context_prop", "enabled")) {
        $thrift_client_handler->addHandler(
          'thrift_context_prop',
          new ThriftContextPropHandler($headers_transport),
        );
      }
    }
  }

}
