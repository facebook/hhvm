<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// message queue

f('ftok', Int64,
  array('pathname' => String,
        'proj' => String));

f('msg_get_queue', Variant,
  array('key' => Int64,
        'perms' => array(Int64, '0666')));

f('msg_send', Boolean,
  array('queue' => Resource,
        'msgtype' => Int64,
        'message' => Variant,
        'serialize' => array(Boolean, 'true'),
        'blocking' => array(Boolean, 'true'),
        'errorcode' => array(Variant | Reference, 'null')));

f('msg_receive', Boolean,
  array('queue' => Resource,
        'desiredmsgtype' => Int64,
        'msgtype' => Int64 | Reference,
        'maxsize' => Int64,
        'message' => Variant | Reference,
        'unserialize' => array(Boolean, 'true'),
        'flags' => array(Int64, '0'),
        'errorcode' => array(Variant | Reference, 'null')));

f('msg_remove_queue', Boolean,
  array('queue' => Resource));

f('msg_set_queue', Boolean,
  array('queue' => Resource,
        'data' => Int64Map));

f('msg_stat_queue', Int64Map,
  array('queue' => Resource));

///////////////////////////////////////////////////////////////////////////////
// semaphore

f('sem_acquire', Boolean,
  array('sem_identifier' => Resource));

f('sem_get', Variant,
  array('key' => Int64,
        'max_acquire' => array(Int64, '1'),
        'perm' => array(Int64, '0666'),
        'auto_release' => array(Boolean, 'true')));

f('sem_release', Boolean,
  array('sem_identifier' => Resource));

f('sem_remove', Boolean,
  array('sem_identifier' => Resource));

///////////////////////////////////////////////////////////////////////////////
// shared memory

f('shm_attach', Variant,
  array('shm_key' => Int64,
        'shm_size' => array(Int64, '10000'),
        'shm_flag' => array(Int64, '0666')));

f('shm_detach', Boolean,
  array('shm_identifier' => Int64));

f('shm_remove', Boolean,
  array('shm_identifier' => Int64));

f('shm_get_var', Variant,
  array('shm_identifier' => Int64,
        'variable_key' => Int64));

f('shm_put_var', Boolean,
  array('shm_identifier' => Int64,
        'variable_key' => Int64,
        'variable' => Any));

f('shm_remove_var', Boolean,
  array('shm_identifier' => Int64,
        'variable_key' => Int64));
