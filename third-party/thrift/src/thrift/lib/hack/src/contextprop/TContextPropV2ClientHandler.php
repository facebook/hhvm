<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

type ClientInstrumentationParams = shape(
  ?'service_name' => string,
  ?'thrift_class' => classname<IThriftClient>,
  ?'client' => IThriftClient,
  ?'fn_name' => string,
  ?'fn_args' => mixed,
  ?'fn_sequence_id' => ?int,
);

final class TContextPropV2ClientHandler extends TClientEventHandler {
  private vec<IContextHandler> $handlers;

  public function __construct(
    private ?TTransportSupportsHeaders $headersTransport,
    private ClientInstrumentationParams $params,
  ) {
    $this->handlers = vec[];
  }

  public function addHandler(IContextHandler $handler): void {
    $this->handlers[] = $handler;
  }

  <<__Override>>
  public function preSend(
    string $fn_name,
    mixed $args,
    int $sequence_id,
    ?string $_service_interface = null,
  ): void {
    try {
      // set params
      $headers_transport = $this->headersTransport;
      if ($headers_transport is null) {
        return;
      }

      // copy TFM so handlers can mutate it
      $tfm_copy = ThriftContextPropState::get()->getTFMCopy();

      $full_params = $this->params;
      $full_params['fn_name'] = $fn_name;
      $full_params['fn_args'] = $args;
      $full_params['fn_sequence_id'] = $sequence_id;

      // call handlers
      foreach ($this->handlers as $handler) {
        $handler->onOutgoingDownstream(
          $full_params,
          $tfm_copy,
          new ImmutableThriftContextPropState(ThriftContextPropState::get()),
        );
      }

      $v = self::encodeTFM($tfm_copy);
      $headers_transport->setWriteHeader(
        ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
        $v,
      );
    } catch (Exception $e) {
      FBLogger('WWW_ContextProp_v2', 'client_handler_preSend')->exception(
        $e,
        "ContextProp v2 client handler preSend threw exception",
      );
    }
  }

  <<__Override>>
  public function preRecv(string $fn_name, ?int $ex_sequence_id): void {
    try {
      // set params
      $full_params = $this->params;
      $full_params['fn_name'] = $fn_name;
      $full_params['fn_sequence_id'] = $ex_sequence_id;

      if ($this->headersTransport is nonnull) {
        $encoded_tfmr = idx(
          $this->headersTransport->getReadHeaders(),
          ThriftFrameworkMetadata_CONSTANTS::ThriftFrameworkMetadataHeaderKey,
        );
        if ($encoded_tfmr is nonnull) {
          $tfmr = self::decodeFrameworkMetadataOnResponse($encoded_tfmr);
          $immutable_tfmr =
            new ImmutableThriftFrameworkMetadataOnResponse($tfmr);
          // call handlers that can mutate ThriftContextPropState
          foreach ($this->handlers as $handler) {
            $handler->onIncomingDownstream(
              ThriftContextPropState::get(),
              $full_params,
              $immutable_tfmr,
            );
          }

        }
      }
    } catch (Exception $e) {
      FBLogger('WWW_ContextProp_v2', 'client_handler_preRecv')->exception(
        $e,
        "ContextProp v2 client handler preRecv threw exception",
      );
    }
  }

  <<__Memoize>>
  private static function decodeFrameworkMetadataOnResponse(
    string $encoded_response_tfm,
  ): ThriftFrameworkMetadataOnResponse {
    $tfmr = ThriftFrameworkMetadataOnResponse::withDefaultValues();
    $tfmr->read(
      new TCompactProtocolAccelerated(
        new TMemoryBuffer(Base64::decode($encoded_response_tfm)),
      ),
    );
    return $tfmr;
  }

  <<__Memoize(#KeyedByIC)>>
  private static function encodeTFM(
    ThriftFrameworkMetadata $tfm,
  )[globals, zoned_shallow]: string {
    $buf = new TMemoryBuffer();
    $proto = new TCompactProtocolAccelerated($buf);
    $tfm->write($proto);
    return Base64::encode($buf->getBuffer());
  }
}
