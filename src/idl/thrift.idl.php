<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// thrift_protocol

f('thrift_protocol_write_binary', NULL,
  array('transportObj' => Object,
        'method_name' => String,
        'msgtype' => Int64,
        'request_struct' => Object,
        'seqid' => Int32,
        'strict_write' => Boolean));

f('thrift_protocol_read_binary', Variant,
  array('transportObj' => Object,
        'obj_typename' => String,
        'strict_read' => Boolean));

dyn('getTransport', 'flush', 'write', 'read');
