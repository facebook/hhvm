<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('oci_connect', Resource,
  array('username' => String,
        'password' => String,
        'db' => array(String, 'null_string'),
        'charset' => array(String, 'null_string'),
        'session_mode' => array(Int32, '0')));

f('oci_new_connect', Resource,
  array('username' => String,
        'password' => String,
        'db' => array(String, 'null_string'),
        'charset' => array(String, 'null_string'),
        'session_mode' => array(Int32, '0')));

f('oci_pconnect', Resource,
  array('username' => String,
        'password' => String,
        'db' => array(String, 'null_string'),
        'charset' => array(String, 'null_string'),
        'session_mode' => array(Int32, '0')));

f('oci_server_version', String,
  array('connection' => Resource));

f('oci_password_change', Variant,
  array('connection' => Variant,
        'username' => String,
        'old_password' => String,
        'new_password' => String));

f('oci_new_cursor', Resource,
  array('connection' => Resource));

f('oci_new_descriptor', Resource,
  array('connection' => Resource,
        'type' => array(Int32, '0')));

f('oci_close', Boolean,
  array('connection' => Resource));

f('oci_commit', Boolean,
  array('connection' => Resource));

f('oci_rollback', Boolean,
  array('connection' => Resource));

f('oci_error', VariantMap,
  array('source' => array(Resource, 'null')));

f('oci_internal_debug', NULL,
  array('onoff' => Boolean));

f('oci_parse', Resource,
  array('connection' => Resource,
        'query' => String));

f('oci_statement_type', String,
  array('statement' => Resource));

f('oci_free_statement', Boolean,
  array('statement' => Resource));

f('oci_free_descriptor', Boolean,
  array('lob' => Resource));

f('oci_bind_array_by_name', Boolean,
  array('statement' => Resource,
        'name' => String,
        'var_array' => VariantMap | Reference,
        'max_table_length' => Int32,
        'max_item_length' => array(Int32, '0'),
        'type' => array(Int32, '0')));

f('oci_bind_by_name', Boolean,
  array('statement' => Resource,
        'ph_name' => String,
        'variable' => Variant | Reference,
        'max_length' => array(Int32, '0'),
        'type' => array(Int32, '0')));

f('oci_cancel', Boolean,
  array('statement' => Resource));

f('oci_define_by_name', Boolean,
  array('statement' => Resource,
        'column_name' => String,
        'variable' => Variant | Reference,
        'type' => array(Int32, '0')));

f('oci_execute', Boolean,
  array('statement' => Resource,
        'mode' => array(Int32, '0')));

f('oci_num_fields', Int32,
  array('statement' => Resource));

f('oci_num_rows', Int32,
  array('statement' => Resource));

f('oci_result', Variant,
  array('statement' => Resource,
        'field' => Variant));

f('oci_set_prefetch', Boolean,
  array('statement' => Resource,
        'rows' => Int32));

f('oci_fetch_all', Int32,
  array('statement' => Resource,
        'output' => VariantMap | Reference,
        'skip' => array(Int32, '0'),
        'maxrows' => array(Int32, '0'),
        'flags' => array(Int32, '0')));

f('oci_fetch_array', Variant,
  array('statement' => Resource,
        'mode' => array(Int32, '0')));

f('oci_fetch_assoc', Variant,
  array('statement' => Resource));

f('oci_fetch_object', Variant,
  array('statement' => Resource));

f('oci_fetch_row', Variant,
  array('statement' => Resource));

f('oci_fetch', Boolean,
  array('statement' => Resource));

f('oci_field_is_null', Boolean,
  array('statement' => Resource,
        'field' => Variant));

f('oci_field_name', String,
  array('statement' => Resource,
        'field' => Int32));

f('oci_field_precision', Int32,
  array('statement' => Resource,
        'field' => Int32));

f('oci_field_scale', Int32,
  array('statement' => Resource,
        'field' => Int32));

f('oci_field_size', Int32,
  array('statement' => Resource,
        'field' => Variant));

f('oci_field_type_raw', Int32,
  array('statement' => Resource,
        'field' => Int32));

f('oci_field_type', Variant,
  array('statement' => Resource,
        'field' => Int32));
