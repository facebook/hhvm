<?hh

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
                                     bool $strict_read): object;

<<__Native>>
function thrift_protocol_read_binary_struct(object $transportobj,
                                            string $obj_typename): mixed;

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
                                      string $obj_typename): mixed;

<<__Native>>
function thrift_protocol_read_compact_struct(object $transportobj,
                                             string $obj_typename): object;
