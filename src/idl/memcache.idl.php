<?php

include_once 'base.php';

pre("#include <libmemcached/memcached.h>");

///////////////////////////////////////////////////////////////////////////////

f('memcache_connect', Object,
  array('host' => String,
        'port' => array(Int32, '0'),
        'timeout' => array(Int32, '0'),
        'timeoutms' => array(Int32, '0')));

f('memcache_pconnect', Object,
  array('host' => String,
        'port' => array(Int32, '0'),
        'timeout' => array(Int32, '0'),
        'timeoutms' => array(Int32, '0')));

f('memcache_add', Boolean,
  array('memcache' => Object,
        'key' => String,
        'var' => Variant,
        'flag' => array(Int32, '0'),
        'expire' => array(Int32, '0')));

f('memcache_set', Boolean,
  array('memcache' => Object,
        'key' => String,
        'var' => Variant,
        'flag' => array(Int32, '0'),
        'expire' => array(Int32, '0')));

f('memcache_replace', Boolean,
  array('memcache' => Object,
        'key' => String,
        'var' => Variant,
        'flag' => array(Int32, '0'),
        'expire' => array(Int32, '0')));

f('memcache_get', Variant,
  array('memcache' => Object,
        'key' => Variant,
        'flags' => array(Int32 | Reference, 'null')));

f('memcache_delete', Boolean,
  array('memcache' => Object,
        'key' => String,
        'expire' => array(Int32, '0')));

f('memcache_increment', Int64,
  array('memcache' => Object,
        'key' => String,
        'offset' => array(Int32, '1')));

f('memcache_decrement', Int64,
  array('memcache' => Object,
        'key' => String,
        'offset' => array(Int32, '1')));

f('memcache_close', Boolean,
  array('memcache' => Object));

f('memcache_debug', Boolean,
  array('onoff' => Boolean));

f('memcache_get_version', Variant,
  array('memcache' => Object));

f('memcache_flush', Boolean,
  array('memcache' => Object,
        'timestamp' => array(Int32, '0')));

f('memcache_setoptimeout', Boolean,
  array('memcache' => Object,
        'timeoutms' => Int32));

f('memcache_get_server_status', Int32,
  array('memcache' => Object,
        'host' => String,
        'port' => array(Int32, '0')));

f('memcache_set_compress_threshold', Boolean,
  array('memcache' => Object,
        'threshold' => Int32,
        'min_savings' => array(Double, '0.2')));

f('memcache_get_stats', VariantMap,
  array('memcache' => Object,
        'type' => array(String, 'null_string'),
        'slabid' => array(Int32, '0'),
        'limit' => array(Int32, '100')));

f('memcache_get_extended_stats', VariantMap,
  array('memcache' => Object,
        'type' => array(String, 'null_string'),
        'slabid' => array(Int32, '0'),
        'limit' => array(Int32, '100')));

f('memcache_set_server_params', Boolean,
  array('memcache' => Object,
        'host' => String,
        'port' => array(Int32, '11211'),
        'timeout' => array(Int32, '0'),
        'retry_interval' => array(Int32, '0'),
        'status' => array(Boolean, 'true'),
        'failure_callback' => array(Variant, 'null_variant')));

f('memcache_add_server', Boolean,
  array('memcache' => Object,
        'host' => String,
        'port' => array(Int32, '11211'),
        'persistent' => array(Boolean, 'false'),
        'weight' => array(Int32, '0'),
        'timeout' => array(Int32, '0'),
        'retry_interval' => array(Int32, '0'),
        'status' => array(Boolean, 'true'),
        'failure_callback' => array(Variant, 'null_variant'),
        'timeoutms' => array(Int32, '0')));


c('Memcache', null, array(),
  array(
    m(PublicMethod, '__construct', null,
      array()),
    m(PublicMethod, 'connect', null,
      array('host' => String,
            'port' => array(Int32, '0'),
            'timeout' => array(Int32, '0'),
            'timeoutms' => array(Int32, '0'))),
    m(PublicMethod, 'pconnect', null,
      array('host' => String,
            'port' => array(Int32, '0'),
            'timeout' => array(Int32, '0'),
            'timeoutms' => array(Int32, '0'))),
    m(PublicMethod, 'add', Boolean,
      array('key' => String,
            'var' => Variant,
            'flag' => array(Int32, '0'),
            'expire' => array(Int32, '0'))),
    m(PublicMethod, 'set', Boolean,
      array('key' => String,
            'var' => Variant,
            'flag' => array(Int32, '0'),
            'expire' => array(Int32, '0'))),
    m(PublicMethod, 'replace', Boolean,
      array('key' => String,
            'var' => Variant,
            'flag' => array(Int32, '0'),
            'expire' => array(Int32, '0'))),
    m(PublicMethod, 'get', Variant,
      array('key' => Variant,
            'flags' => array(Int32 | Reference, 'null'))),
    m(PublicMethod, 'delete', Boolean,
      array('key' => String,
            'expire' => array(Int32, '0'))),
    m(PublicMethod, 'increment', Int64,
      array('key' => String,
            'offset' => array(Int32, '1'))),
    m(PublicMethod, 'decrement', Int64,
      array('key' => String,
            'offset' => array(Int32, '1'))),
    m(PublicMethod, 'getversion', Variant
    ),
    m(PublicMethod, 'flush', Boolean,
      array('expire' => array(Int32, '0'))),
    m(PublicMethod, 'setoptimeout', Boolean,
      array('timeoutms' => Int64)),
    m(PublicMethod, 'close', Boolean),
    m(PublicMethod, 'getserverstatus', Int32,
      array('host' => String,
            'port' => array(Int32, '0'))),
    m(PublicMethod, 'setcompressthreshold', Boolean,
      array('threshold' => Int32,
            'min_savings' => array(Double, '0.2'))),
    m(PublicMethod, 'getstats', VariantMap,
      array('type' => array(String, 'null_string'),
            'slabid' => array(Int32, '0'),
            'limit' => array(Int32, '100'))),
    m(PublicMethod, 'getextendedstats', VariantMap,
      array('type' => array(String, 'null_string'),
            'slabid' => array(Int32, '0'),
            'limit' => array(Int32, '100'))),
    m(PublicMethod, 'setserverparams', Boolean,
      array('host' => String,
            'port' => array(Int32, '11211'),
            'timeout' => array(Int32, '0'),
            'retry_interval' => array(Int32, '0'),
            'status' => array(Boolean, 'true'),
            'failure_callback' => array(Variant, 'null_variant'))),
    m(PublicMethod, 'addserver', Boolean,
      array('host' => String,
            'port' => array(Int32, '11211'),
            'persistent' => array(Boolean, 'false'),
            'weight' => array(Int32, '0'),
            'timeout' => array(Int32, '0'),
            'retry_interval' => array(Int32, '0'),
            'status' => array(Boolean, 'true'),
            'failure_callback' => array(Variant, 'null_variant'),
            'timeoutms' => array(Int32, '0'))),
    ),
  array(),
  "\n".
  " private:\n".
  "  memcached_st m_memcache;\n".
  "  int m_compress_threshold;\n".
  "  double m_min_compress_savings;"
  );

/*
f('memcache_add_server', Boolean,
  array('x' => x));
f('memcache_set_server_params', Boolean,
  array('x' => x));
*/
