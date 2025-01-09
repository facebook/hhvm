<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('xdc_artillery')>>
final class ExperimentIdContextHandlerTest extends WWWTest {

  use ClassLevelTest;

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    ThriftContextPropState::get()->clear();
  }

  public async function testExperimentIdsAddedToCtx(): Awaitable<void> {
    $expected_experiment_ids = vec[123, 456];
    $params = shape();
    $handler = new ExperimentIdContextHandler();
    $mutable_ctx = ThriftContextPropState::get();

    $tfmr = ThriftFrameworkMetadataOnResponse::fromShape(
      shape(
        'experiment_ids' => ExperimentIdsUpdate::fromShape(shape(
          'merge' => $expected_experiment_ids,
        )),
      ),
    );

    $immutable_tfmr = new ImmutableThriftFrameworkMetadataOnResponse($tfmr);

    $original_expirment_ids_from_tfm = $immutable_tfmr->getExperimentIds();

    $handler->onIncomingDownstream($mutable_ctx, $params, $immutable_tfmr);
    // handler should NOT change the TFM
    $experiment_ids_from_tfm = $immutable_tfmr->getExperimentIds();
    expect($experiment_ids_from_tfm)->toEqual($original_expirment_ids_from_tfm);
    // handler should change the experiment_ids field ThriftContextPropState
    $experiment_ids_from_ctx = $mutable_ctx->getExperimentIds();
    expect($experiment_ids_from_ctx)->toEqual($expected_experiment_ids);
  }

  public async function testEmptyExperimentIdsNotAddedToTfm(): Awaitable<void> {
    $expected_experiment_ids = vec[];
    $params = shape();
    $handler = new ExperimentIdContextHandler();
    $mutable_ctx = ThriftContextPropState::get();

    $tfmr = ThriftFrameworkMetadataOnResponse::fromShape(
      shape(
        'experiment_ids' => ExperimentIdsUpdate::fromShape(shape(
          'merge' => $expected_experiment_ids,
        )),
      ),
    );

    $immutable_tfmr = new ImmutableThriftFrameworkMetadataOnResponse($tfmr);

    $original_expirment_ids_from_tfm = $immutable_tfmr->getExperimentIds();

    $handler->onIncomingDownstream($mutable_ctx, $params, $immutable_tfmr);
    // handler should NOT change the TFM
    $experiment_ids_from_tfm = $immutable_tfmr->getExperimentIds();
    expect($experiment_ids_from_tfm)->toEqual($original_expirment_ids_from_tfm);
    // handler should change the experiment_ids field ThriftContextPropState
    $experiment_ids_from_ctx = $mutable_ctx->getExperimentIds();
    expect($experiment_ids_from_ctx)->toEqual($expected_experiment_ids);
  }

  public async function testRegisterClientHandler(): Awaitable<void> {
    $handler = new ExperimentIdContextHandler();
    $expected_experiment_ids = vec[123, 456];

    $params = shape();
    $transport =
      TServiceRouterTransport::create("test_service", dict[], dict[]);

    // create encoded TFMR with experiment ids set
    $tfmr = ThriftFrameworkMetadataOnResponse::fromShape(
      shape(
        'experiment_ids' => ExperimentIdsUpdate::fromShape(shape(
          'merge' => $expected_experiment_ids,
        )),
      ),
    );
    $encoded_tfmr = self::encodeTFMR($tfmr);

    $transport->setReadHeaders(Map {
      ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey =>
        $encoded_tfmr,
    });
    $client_handler = new TContextPropV2ClientHandler($transport, $params);
    $client_handler->addHandler($handler);

    $client_handler->preRecv('function_name', 0);
    // assert that ThriftContextPropState has experiment_ids
    $experiment_ids_from_cp_state =
      ThriftContextPropState::get()->getExperimentIds();

    expect($experiment_ids_from_cp_state)->toEqual($expected_experiment_ids);
  }

  private static function encodeTFMR(
    ThriftFrameworkMetadataOnResponse $tfmr,
  ): string {
    $buf = new TMemoryBuffer();
    $proto = new TCompactProtocolAccelerated($buf);
    $tfmr->write($proto);
    return Base64::encode($buf->getBuffer());
  }

  public async function testExperimentIdsAddedToServerOutgoingResponse(
  ): Awaitable<void> {
    $context_prop_state = ThriftContextPropState::get();
    $context_prop_state->setExperimentIds(vec[123, 456]);

    $mutable_tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $immutable_ctx = new ImmutableThriftContextPropState($context_prop_state);
    $params = shape();
    $handler = new ExperimentIdContextHandler();

    $handler->onOutgoingUpstream($params, $mutable_tfmr, $immutable_ctx);
    // handler should change the experiment_ids field ThriftFrameworkMetadataOnResponse
    $experiment_ids_from_tfm = $mutable_tfmr->experiment_ids?->get_merge();
    expect($experiment_ids_from_tfm)->toEqual(vec[123, 456]);
  }

  public async function testEmptyExperimentIdsNotAddedToServerOutgoingResponse(
  ): Awaitable<void> {
    $context_prop_state = ThriftContextPropState::get();
    $context_prop_state->setExperimentIds(vec[]);

    $mutable_tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $immutable_ctx = new ImmutableThriftContextPropState($context_prop_state);
    $params = shape();
    $handler = new ExperimentIdContextHandler();

    $handler->onOutgoingUpstream($params, $mutable_tfmr, $immutable_ctx);
    // handler should change the experiment_ids field ThriftFrameworkMetadataOnResponse
    $experiment_ids_from_tfm = $mutable_tfmr->experiment_ids?->get_merge();
    expect($experiment_ids_from_tfm)->toBeNull();
  }
}
