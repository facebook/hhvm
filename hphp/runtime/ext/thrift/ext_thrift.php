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
  public function __toString(): string;
}
