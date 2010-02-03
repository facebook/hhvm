<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('dangling_server_proxy_old_request', Boolean);

f('dangling_server_proxy_new_request', Boolean,
  array('host' => String));

///////////////////////////////////////////////////////////////////////////////

f('pagelet_server_task_start', Resource,
  array('url' => String,
        'headers' => array(StringMap, 'null_array'),
        'post_data' => array(String, 'null_string')));

f('pagelet_server_task_status', Boolean,
  array('task' => Resource));

f('pagelet_server_task_result', String,
  array('task' => Resource,
        'headers' => StringVec | Reference,
        'code' => Int64 | Reference));

///////////////////////////////////////////////////////////////////////////////

f('xbox_send_message', Boolean,
  array('msg' => String,
        'ret' => Variant | Reference,
        'timeout_ms' => Int64,
        'host' => array(String, '"localhost"')));

f('xbox_post_message', Boolean,
  array('msg' => String,
        'host' => array(String, '"localhost"')));

f('xbox_task_start', Resource,
  array('message' => String));

f('xbox_task_status', Boolean,
  array('task' => Resource));

f('xbox_task_result', Int64,
  array('task' => Resource,
        'timeout_ms' => Int64,
        'ret' => Variant | Reference));
