<?hh // partial

<<__Native>>
function thrift_protocol_write_binary(object $transportobj,
                                      string $method_name,
                                      int $msgtype,
                                      object $request_struct,
                                      int $seqid,
                                      bool $strict_write,
                                      bool $oneway = false): void;

<<__Native>>
function thrift_protocol_read_binary(object $transportobj,
                                     string $obj_typename,
                                     bool $strict_read,
                                     int $options = 0): object;

<<__Native>>
function thrift_protocol_read_binary_struct(object $transportobj,
                                            string $obj_typename,
                                            int $options = 0): mixed;

<<__Native>>
function thrift_protocol_set_compact_version(int $version)[leak_safe]: int;

<<__Native>>
function thrift_protocol_write_compact(object $transportobj,
                                       string $method_name,
                                       int $msgtype,
                                       object $request_struct,
                                       int $seqid,
                                       bool $oneway = false): void;

<<__Native>>
function thrift_protocol_read_compact(object $transportobj,
                                      string $obj_typename,
                                      int $options = 0): mixed;

<<__Native>>
function thrift_protocol_read_compact_struct(object $transportobj,
                                             string $obj_typename,
                                             int $options = 0): object;

<<__NativeData("InteractionId")>>
class InteractionId {
  private function __construct()[]: void {}
}

<<__NativeData("RpcOptions")>>
final class RpcOptions implements IPureStringishObject {
  public function __construct()[]: void {}

  <<__Native>>
  public function setChunkBufferSize(int $chunk_buffer_size)[write_props]: RpcOptions;

  <<__Native>>
  public function setRoutingKey(string $routing_key)[write_props]: RpcOptions;

  <<__Native>>
  public function setShardId(string $shard_id)[write_props]: RpcOptions;

  <<__Native>>
  public function setWriteHeader(string $key, string $value)[write_props]: RpcOptions;

  <<__Native>>
  public function setHeader(string $key, string $value)[write_props]: RpcOptions;

  <<__Native>>
  public function setLoggingContext(string $logging_metadata)[write_props]: RpcOptions;

  <<__Native>>
  public function setOverallTimeout(int $overall_timeout)[write_props]: RpcOptions;

  <<__Native>>
  public function setProcessingTimeout(int $processing_timeout)[write_props]: RpcOptions;

  <<__Native>>
  public function setChunkTimeout(int $chunk_timeout)[write_props]: RpcOptions;

  <<__Native>>
  public function setInteractionId(InteractionId $interaction_id)[write_props]: RpcOptions;

  <<__Native>>
  public function setFaultToInject(string $key, string $value)[write_props]: RpcOptions;

  <<__Native>>
  public function setSerializedAuthProofs(string $serializedTokenData)[write_props]: RpcOptions;

  <<__Native>>
  public function __toString()[]: string;
}

<<__NativeData("TClientBufferedStream")>>
final class TClientBufferedStream {
  public function __construct(): void {}

  public async function gen<TStreamResponse>(
    (function(?string, ?Exception): TStreamResponse) $streamDecode,
  ): HH\AsyncGenerator<null, TStreamResponse, void> {
    while (true) {
      $timer = WallTimeOperation::begin();
      try {
        list($buffer, $ex_msg) = await $this->genNext();
      } catch (Exception $ex) {
        $streamDecode(null, $ex);
        break;
      } finally {
        $timer->end();
      }

      if ($buffer === null && $ex_msg === null) {
        // If still no data after buffer filling, the stream has finished
        break;
      }
      if ($buffer !== null) {
        foreach ($buffer as $value) {
          yield $streamDecode($value, null);
        }
      }
      if ($ex_msg !== null) {
        $streamDecode(
          null,
          new TApplicationException($ex_msg, TApplicationException::UNKNOWN),
        );
        break;
      }
    }
  }

  <<__Native>>
  public function genNext(): Awaitable<(?vec<string>,?string)>;
}

<<__NativeData("TClientSink")>>
final class TClientSink {
  public function __construct(): void {}

  public async function genCreditsOrFinalResponseHelper(
  ): Awaitable<(int, ?string, ?Exception)> {
    $timer = WallTimeOperation::begin();
    try {
      list($credits, $final_response, $exception) =
        await $this->genCreditsOrFinalResponse();
    } finally {
      $timer->end();
    }
    if (
      ($credits === null || $credits === 0) &&
      $final_response === null &&
      $exception === null
    ) {
      $exception = "No credits or final response received";
    }
    return tuple(
      $credits,
      $final_response,
      $exception !== null
        ? new TApplicationException($exception, TApplicationException::UNKNOWN)
        : null,
    );
  }

  public async function genSink<TSinkType, TFinalResponseType>(
    HH\AsyncGenerator<null, TSinkType, void> $payload_generator,
    (function(?TSinkType, ?Exception): (string, ?bool)) $payloadEncode,
    (function(?string, ?Exception): TFinalResponseType) $finalResponseDecode,
  ): Awaitable<TFinalResponseType> {
    $shouldContinue = true;
    while (true) {
      list($credits, $final_response, $exception) =
        await $this->genCreditsOrFinalResponseHelper();
      if ($final_response !== null || $exception !== null) {
        break;
      }
      if ($credits > 0 && $shouldContinue) {
        try {
          foreach ($payload_generator await as $pld) {
            list($encoded_str, $_) = $payloadEncode($pld, null);
            $shouldContinue = $this->sendPayloadOrSinkComplete($encoded_str);
            $credits--;
            if ($credits === 0 || !$shouldContinue) {
              break;
            }
          }
        } catch (Exception $ex) {
          // If async generator throws any error,
          // exception should be encoded and sent to server before throwing
          list($encoded_ex, $is_application_ex) = $payloadEncode(null, $ex);
          $this->sendClientException(
            $encoded_ex,
            $is_application_ex ? $ex->getMessage() : null,
          );
          // send exception back to the client, don't wait for final response
          throw $ex;
        }
        // If $credits > 0 and $shouldContinue = true,
        // then that means async generator has finished
        // and we should send sink complete.
        if ($credits > 0 && $shouldContinue) {
          $this->sendPayloadOrSinkComplete(null);
          $shouldContinue = false;
        }
      }
    }
    return $finalResponseDecode($final_response, $exception);
  }

  // Returns false when server has cancelled or sink complete is sent
  <<__Native>>
  public function sendPayloadOrSinkComplete(?string $payload): bool;

  <<__Native>>
  public function genCreditsOrFinalResponse(
  ): Awaitable<?(int, ?string, ?string)>;

  <<__Native>>
  public function sendClientException(
    string $ex_encoded_string,
    ?string $ex_msg,
  ): void;
}
