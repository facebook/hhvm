<?hh

<<__Native>>
function thrift_protocol_write_binary(mixed $transportobj,
                                      string $method_name,
                                      int $msgtype,
                                      mixed $request_struct,
                                      int $seqid,
                                      bool $strict_write,
                                      bool $oneway = false): void;

<<__Native>>
function thrift_protocol_read_binary(mixed $transportobj,
                                     string $obj_typename,
                                     bool $strict_read): mixed;

<<__Native>>
function thrift_protocol_read_binary_struct(mixed $transportobj,
                                            string $obj_typename): mixed;

<<__Native>>
function thrift_protocol_set_compact_version(int $version): int;

<<__Native>>
function thrift_protocol_write_compact(mixed $transportobj,
                                       string $method_name,
                                       int $msgtype,
                                       mixed $request_struct,
                                       int $seqid,
                                       bool $oneway = false): void;

<<__Native>>
function thrift_protocol_read_compact(mixed $transportobj,
                                      string $obj_typename): mixed;

<<__Native>>
function thrift_protocol_read_compact_struct(mixed $transportobj,
                                             string $obj_typename): mixed;
