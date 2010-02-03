<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('apache_child_terminate', Boolean);
f('apache_get_modules', StringVec);
f('apache_get_version', String);
f('apache_getenv', String,
  array('variable' => String,
        'walk_to_top' => array(Boolean, 'false')));
f('apache_lookup_uri', Object,
  array('filename' => String));
f('apache_note', Variant,
  array('note_name' => String,
        'note_value' => array(String, 'null_string')));
f('apache_request_headers', StringVec);
f('apache_reset_timeout', Boolean);
f('apache_response_headers', StringVec);
f('apache_setenv', Boolean,
  array('variable' => String,
        'value' => String,
        'walk_to_top' => array(Boolean, 'false')));
f('ascii2ebcdic', Int32,
  array('ascii_str' => String));
f('ebcdic2ascii', Int32,
  array('ebcdic_str' => String));
f('getallheaders', StringVec);
f('virtual', Boolean,
  array('filename' => String));

f('apache_get_config', Variant);
f('apache_get_scoreboard', Variant);
f('apache_get_rewrite_rules', Variant);
