<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// stream functions

f('stream_context_create', Resource,
  array('options' => array(VariantMap, 'null_array'),
        'params' => array(VariantMap, 'null_array')));

f('stream_context_get_default', Resource,
  array('options' => array(VariantMap, 'null_array')));

f('stream_context_get_options', VariantMap,
  array('stream_or_context' => Resource));

f('stream_context_set_option', Boolean,
  array('stream_or_context' => Resource,
        'wrapper' => Variant,
        'option' => array(String, 'null_string'),
        'value' => array(Variant, 'null_variant')));

f('stream_context_set_param', Boolean,
  array('stream_or_context' => Resource,
        'params' => VariantMap));

f('stream_copy_to_stream', Variant,
  array('source' => Resource,
        'dest' => Resource,
        'maxlength' => array(Int32, '0'),
        'offset' => array(Int32, '0')));

f('stream_encoding', Boolean,
  array('stream' => Resource,
        'encoding' => array(String, 'null_string')));

f('stream_bucket_append', NULL,
  array('brigade' => Resource,
        'bucket' => Resource));

f('stream_bucket_prepend', NULL,
  array('brigade' => Resource,
        'bucket' => Resource));

f('stream_bucket_make_writeable', Resource,
  array('brigade' => Resource));

f('stream_bucket_new', Resource,
  array('stream' => Resource,
        'buffer' => String));

f('stream_filter_register', Boolean,
  array('filtername' => String,
        'classname' => String));

f('stream_filter_remove', Boolean,
  array('stream_filter' => Resource));

f('stream_filter_append', Resource,
  array('stream' => Resource,
        'filtername' => String,
        'read_write' => array(Int32, '0'),
        'params' => array(Variant, 'null_variant')));

f('stream_filter_prepend', Resource,
  array('stream' => Resource,
        'filtername' => String,
        'read_write' => array(Int32, '0'),
        'params' => array(Variant, 'null_variant')));

f('stream_get_contents', Variant,
  array('handle' => Resource,
        'maxlen' => array(Int32, '0'),
        'offset' => array(Int32, '0')));

f('stream_get_filters', StringVec);

f('stream_get_line', Variant,
  array('handle' => Resource,
        'length' => array(Int32, '0'),
        'ending' => array(String, 'null_string')));

f('stream_get_meta_data', VariantMap,
  array('stream' => Resource));

f('stream_get_transports', StringVec);

f('stream_get_wrappers', StringVec);

f('stream_register_wrapper', Boolean,
  array('protocol' => String,
        'classname' => String));

f('stream_wrapper_register', Boolean,
  array('protocol' => String,
        'classname' => String));

f('stream_wrapper_restore', Boolean,
  array('protocol' => String));

f('stream_wrapper_unregister', Boolean,
  array('protocol' => String));

f('stream_resolve_include_path', String,
  array('filename' => String,
        'context' => array(Resource, 'null_object')));

f('stream_select', Variant,
  array('read' => VariantVec | Reference,
        'write' => VariantVec | Reference,
        'except' => VariantVec | Reference,
        'vtv_sec' => Variant,
        'tv_usec' => array(Int32, '0')));

f('stream_set_blocking', Boolean,
  array('stream' => Resource,
        'mode' => Int32));

f('stream_set_timeout', Boolean,
  array('stream' => Resource,
        'seconds' => Int32,
        'microseconds' => array(Int32, '0')));

f('stream_set_write_buffer', Int32,
  array('stream' => Resource,
        'buffer' => Int32));

f('set_file_buffer', Int32,
  array('stream' => Resource,
        'buffer' => Int32));

///////////////////////////////////////////////////////////////////////////////
// socket functions

f('stream_socket_accept', Variant,
  array('server_socket' => Resource,
        'timeout' => array(Double, '0.0'),
        'peername' => array(String | Reference, 'null')));

f('stream_socket_server', Variant,
  array('local_socket' => String,
        'errnum' => array(Int32 | Reference, 'null'),
        'errstr' => array(String | Reference, 'null'),
        'flags' => array(Int32, '0'),
        'context' => array(Resource, 'null_object')));

f('stream_socket_client', Variant,
  array('remote_socket' => String,
        'errnum' => array(Int32 | Reference, 'null'),
        'errstr' => array(String | Reference, 'null'),
        'timeout' => array(Double, '0.0'),
        'flags' => array(Int32, '0'),
        'context' => array(Resource, 'null_object')));

f('stream_socket_enable_crypto', Variant,
  array('stream' => Resource,
        'enable' => Boolean,
        'crypto_type' => array(Int32, '0'),
        'session_stream' => array(Resource, 'null_object')));

f('stream_socket_get_name', Variant,
  array('handle' => Resource,
        'want_peer' => Boolean));

f('stream_socket_pair', Variant,
  array('domain' => Int32,
        'type' => Int32,
        'protocol' => Int32));

f('stream_socket_recvfrom', Variant,
  array('socket' => Resource,
        'length' => Int32,
        'flags' => array(Int32, '0'),
        'address' => array(String, 'null_string')));

f('stream_socket_sendto', Variant,
  array('socket' => Resource,
        'data' => String,
        'flags' => array(Int32, '0'),
        'address' => array(String, 'null_string')));

f('stream_socket_shutdown', Boolean,
  array('stream' => Resource,
        'how' => Int32));

