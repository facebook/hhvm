<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// DNS

f('gethostbyaddr', Variant,
  array('ip_address' => String));

f('gethostbyname', String,
  array('hostname' => String));

f('gethostbynamel', Variant,
  array('hostname' => String));

f('getprotobyname', Variant,
  array('name' => String));

f('getprotobynumber', Variant,
  array('number' => Int32));

f('getservbyname', Variant,
  array('service' => String,
        'protocol' => String));

f('getservbyport', Variant,
  array('port' => Int32,
        'protocol' => String));

f('inet_ntop', Variant,
  array('in_addr' => String));

f('inet_pton', Variant,
  array('address' => String));

f('ip2long', Variant,
  array('ip_address' => String));

f('long2ip', String,
  array('proper_address' => Int32));

f('dns_check_record', Boolean,
  array('host' => String,
        'type' => array(String, 'null_string')));

f('checkdnsrr', Boolean,
  array('host' => String,
        'type' => array(String, 'null_string')));

f('dns_get_record', Variant,
  array('hostname' => String,
        'type' => array(Int32, '-1'),
        'authns' => array(VariantMap | Reference, 'null'),
        'addtl' => array(VariantMap | Reference, 'null')));

f('dns_get_mx', Boolean,
  array('hostname' => String,
        'mxhosts' => VariantMap | Reference,
        'weights' => array(VariantMap | Reference, 'null')));

f('getmxrr', Boolean,
  array('hostname' => String,
        'mxhosts' => VariantMap | Reference,
        'weight' => array(VariantMap | Reference, 'null')));

///////////////////////////////////////////////////////////////////////////////
// socket

f('fsockopen', Variant,
  array('hostname' => String,
        'port' => array(Int32, '-1'),
        'errnum' => array(Int32 | Reference, 'null'),
        'errstr' => array(String | Reference, 'null'),
        'timeout' => array(Double, '0.0')));

f('pfsockopen', Variant,
  array('hostname' => String,
        'port' => array(Int32, '-1'),
        'errnum' => array(Int32 | Reference, 'null'),
        'errstr' => array(String | Reference, 'null'),
        'timeout' => array(Double, '0.0')));

f('socket_get_status', VariantMap,
  array('stream' => Resource));

f('socket_set_blocking', Boolean,
  array('stream' => Resource,
        'mode' => Int32));

f('socket_set_timeout', Boolean,
  array('stream' => Resource,
        'seconds' => Int32,
        'microseconds' => array(Int32, '0')));

///////////////////////////////////////////////////////////////////////////////
// http

f('header', NULL,
  array('str' => String,
        'replace' => array(Boolean, 'true'),
        'http_response_code' => array(Int32, '0')));

f('headers_list', StringVec);

f('headers_sent', Boolean,
  array('file' => array(String | Reference, 'null'),
        'line' => array(Int32 | Reference, 'null')));

f('header_remove', NULL,
  array('name' => array(String, 'null_string')));

f('setcookie', Boolean,
  array('name' => String,
        'value' => array(String, 'null_string'),
        'expire' => array(Int32, '0'),
        'path' => array(String, 'null_string'),
        'domain' => array(String, 'null_string'),
        'secure' => array(Boolean, 'false'),
        'httponly' => array(Boolean, 'false')));

f('setrawcookie', Boolean,
  array('name' => String,
        'value' => array(String, 'null_string'),
        'expire' => array(Int32, '0'),
        'path' => array(String, 'null_string'),
        'domain' => array(String, 'null_string'),
        'secure' => array(Boolean, 'false'),
        'httponly' => array(Boolean, 'false')));

///////////////////////////////////////////////////////////////////////////////
// syslog

f('define_syslog_variables');

f('openlog', NULL,
  array('ident' => String,
        'option' => Int32,
        'facility' => Int32));

f('closelog');

f('syslog', NULL,
  array('priority' => Int32,
        'message' => String));
