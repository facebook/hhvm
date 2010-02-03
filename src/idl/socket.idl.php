<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('socket_create', Variant,
  array('domain' => Int32,
        'type' => Int32,
        'protocol' => Int32));

f('socket_create_listen', Variant,
  array('port' => Int32,
        'backlog' => array(Int32, '128')));

f('socket_create_pair', Boolean,
  array('domain' => Int32,
        'type' => Int32,
        'protocol' => Int32,
        'fd' => Int64Vec | Reference));

f('socket_get_option', Variant,
  array('socket' => Resource,
        'level' => Int32,
        'optname' => Int32));

f('socket_getpeername', Boolean,
  array('socket' => Resource,
        'address' => String | Reference,
        'port' => array(Int32 | Reference, 'null')));

f('socket_getsockname', Boolean,
  array('socket' => Resource,
        'address' => String | Reference,
        'port' => array(Int32 | Reference, 'null')));

f('socket_set_block', Boolean,
  array('socket' => Resource));

f('socket_set_nonblock', Boolean,
  array('socket' => Resource));

f('socket_set_option', Boolean,
  array('socket' => Resource,
        'level' => Int32,
        'optname' => Int32,
        'optval' => Variant));

f('socket_connect', Boolean,
  array('socket' => Resource,
        'address' => String,
        'port' => array(Int32, '0')));

f('socket_bind', Boolean,
  array('socket' => Resource,
        'address' => String,
        'port' => array(Int32, '0')));

f('socket_listen', Boolean,
  array('socket' => Resource,
        'backlog' => array(Int32, '0')));

f('socket_select', Variant,
  array('read' => VariantVec | Reference,
        'write' => VariantVec | Reference,
        'except' => VariantVec | Reference,
        'vtv_sec' => Variant,
        'tv_usec' => array(Int32, '0')));

f('socket_server', Variant,
  array('hostname' => String,
        'port' => array(Int32, '-1'),
        'errnum' => array(Int32 | Reference, 'null'),
        'errstr' => array(String | Reference, 'null')));

f('socket_accept', Variant,
  array('socket' => Resource));

f('socket_read', Variant,
  array('socket' => Resource,
        'length' => Int32,
        'type' => array(Int32, '0')));

f('socket_write', Variant,
  array('socket' => Resource,
        'buffer' => String,
        'length' => array(Int32, '0')));

f('socket_send', Variant,
  array('socket' => Resource,
        'buf' => String,
        'len' => Int32,
        'flags' => Int32));

f('socket_sendto', Variant,
  array('socket' => Resource,
        'buf' => String,
        'len' => Int32,
        'flags' => Int32,
        'addr' => String,
        'port' => array(Int32, '0')));

f('socket_recv', Variant,
  array('socket' => Resource,
        'buf' => String | Reference,
        'len' => Int32,
        'flags' => Int32));

f('socket_recvfrom', Variant,
  array('socket' => Resource,
        'buf' => String | Reference,
        'len' => Int32,
        'flags' => Int32,
        'name' => String | Reference,
        'port' => array(Int32 | Reference, '0')));

f('socket_shutdown', Boolean,
  array('socket' => Resource,
        'how' => array(Int32, '0')));

f('socket_close', NULL,
  array('socket' => Resource));

f('socket_strerror', String,
  array('errnum' => Int32));

f('socket_last_error', Int32,
  array('socket' => array(Resource, 'null_object')));

f('socket_clear_error', NULL,
  array('socket' => array(Resource, 'null_object')));

