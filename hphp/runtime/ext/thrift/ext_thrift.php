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
function thrift_protocol_set_compact_version(int $version): int;

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
  private function __construct(): void {}
}

<<__NativeData("RpcOptions")>>
final class RpcOptions {
  public function __construct(): void {}

  /* Shared empty object for default use, e.g. in a generated code. */
  <<__Memoize>>
  public static function getSharedEmpty(): RpcOptions {
    return new RpcOptions();
  }

  <<__Native>>
  public function setChunkBufferSize(int $chunk_buffer_size): RpcOptions;

  <<__Native>>
  public function setRoutingKey(string $routing_key): RpcOptions;

  <<__Native>>
  public function setShardId(string $shard_id): RpcOptions;

  <<__Native>>
  public function setWriteHeader(string $key, string $value): RpcOptions;

  <<__Native>>
  public function setHeader(string $key, string $value): RpcOptions;

  <<__Native>>
  public function setLoggingContext(string $logging_metadata): RpcOptions;

  <<__Native>>
  public function setOverallTimeout(int $overall_timeout): RpcOptions;

  <<__Native>>
  public function setProcessingTimeout(int $processing_timeout): RpcOptions;

  <<__Native>>
  public function setInteractionId(InteractionId $interaction_id): RpcOptions;

  <<__Native>>
  public function __toString(): string;
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
