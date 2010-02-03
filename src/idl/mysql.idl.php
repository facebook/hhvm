<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('mysql_connect', Variant,
  array('server' => array(String, 'null_string'),
        'username' => array(String, 'null_string'),
        'password' => array(String, 'null_string'),
        'new_link' => array(Boolean, 'false'),
        'client_flags' => array(Int32, '0'),
        'connect_timeout_ms' => array(Int32, '-1'),
        'query_timeout_ms' => array(Int32, '-1')));

f('mysql_pconnect', Variant,
  array('server' => array(String, 'null_string'),
        'username' => array(String, 'null_string'),
        'password' => array(String, 'null_string'),
        'client_flags' => array(Int32, '0'),
        'connect_timeout_ms' => array(Int32, '-1'),
        'query_timeout_ms' => array(Int32, '-1')));

f('mysql_set_charset', Variant,
  array('charset' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_ping', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_escape_string', String,
  array('unescaped_string' => String));

f('mysql_real_escape_string', Variant,
  array('unescaped_string' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_client_encoding', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_close', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_errno', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_error', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_get_client_info', String);

f('mysql_get_host_info', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_get_proto_info', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_get_server_info', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_info', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_insert_id', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_stat', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_thread_id', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_create_db', Variant,
  array('db' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_select_db', Variant,
  array('db' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_drop_db', Variant,
  array('db' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_affected_rows', Variant,
  array('link_identifier' => array(Variant, 'null')));

///////////////////////////////////////////////////////////////////////////////
// query functions

f('mysql_set_timeout', Boolean,
  array('query_timeout_ms' => array(Int32, '-1'),
        'link_identifier' => array(Variant, 'null')));

f('mysql_query', Variant,
  array('query' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_unbuffered_query', Variant,
  array('query' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_db_query', Variant,
  array('database' => String,
        'query' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_list_dbs', Variant,
  array('link_identifier' => array(Variant, 'null')));

f('mysql_list_tables', Variant,
  array('database' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_list_fields', Variant,
  array('database_name' => String,
        'table_name' => String,
        'link_identifier' => array(Variant, 'null')));

f('mysql_list_processes', Variant,
  array('link_identifier' => array(Variant, 'null')));

///////////////////////////////////////////////////////////////////////////////
// result functions

f('mysql_db_name', Variant,
  array('result' => Variant,
        'row' => Int32,
        'field' => array(Variant, 'null_variant')));

f('mysql_tablename', Variant,
  array('result' => Variant,
        'i' => Int32));

f('mysql_num_fields', Variant,
  array('result' => Variant));

f('mysql_num_rows', Variant,
  array('result' => Variant));

f('mysql_free_result', Variant,
  array('result' => Variant));

///////////////////////////////////////////////////////////////////////////////
// row operations

f('mysql_data_seek', Boolean,
  array('result' => Variant,
        'row' => Int32));

f('mysql_fetch_row', Variant,
  array('result' => Variant));

f('mysql_fetch_assoc', Variant,
  array('result' => Variant));

f('mysql_fetch_array', Variant,
  array('result' => Variant,
        'result_type' => array(Int32, '3')));

f('mysql_fetch_lengths', Variant,
  array('result' => Variant));

f('mysql_fetch_object', Variant,
  array('result' => Variant,
        'class_name' => array(String, '"stdClass"'),
        'params' => array(VariantVec, 'null')));

f('mysql_result', Variant,
  array('result' => Variant,
        'row' => Int32,
        'field' => array(Variant, 'null_variant')));

///////////////////////////////////////////////////////////////////////////////
// field info

f('mysql_fetch_field', Variant,
  array('result' => Variant,
        'field' => array(Int32, '-1')));

f('mysql_field_seek', Boolean,
  array('result' => Variant,
        'field' => array(Int32, '0')));

f('mysql_field_name', Variant,
  array('result' => Variant,
        'field' => array(Int32, '0')));

f('mysql_field_table', Variant,
  array('result' => Variant,
        'field' => array(Int32, '0')));

f('mysql_field_len', Variant,
  array('result' => Variant,
        'field' => array(Int32, '0')));

f('mysql_field_type', Variant,
  array('result' => Variant,
        'field' => array(Int32, '0')));

f('mysql_field_flags', Variant,
  array('result' => Variant,
        'field' => array(Int32, '0')));
