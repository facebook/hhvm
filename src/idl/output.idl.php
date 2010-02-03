<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('ob_start', Boolean,
  array('output_callback' => array(Variant, 'null'),
        'chunk_size' => array(Int32, '0'),
        'erase' => array(Boolean, 'true')));

f('ob_clean');

f('ob_flush');

f('ob_end_clean', Boolean);

f('ob_end_flush', Boolean);

f('flush');

f('ob_get_clean', String);
f('ob_get_contents', String);
f('ob_get_flush', String);
f('ob_get_length', Int32);
f('ob_get_level', Int32);
f('ob_get_status', VariantMap,
  array('full_status' => array(Boolean, 'false')));

f('ob_gzhandler', String,
  array('buffer' => String,
        'mode' => Int32));

f('ob_implicit_flush', NULL,
  array('flag' => array(Boolean, 'true')));

f('ob_list_handlers', StringVec);

f('output_add_rewrite_var', Boolean,
  array('name' => String,
        'value' => String));

f('output_reset_rewrite_vars', Boolean);

f('hphp_log', Boolean,
  array('filename' => String,
        'message' =>String));

f('hphp_stats', NULL,
  array('name' => String,
        'value' => Int64));

f('hphp_get_stats', Int64,
  array('name' => String));

f('hphp_output_global_state', NULL,
   array('filename' => array(String, 'null_string')));
