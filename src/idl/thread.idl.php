<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// service thread functions

f('hphp_service_thread_started', NULL);

// thread info functions
f('hphp_thread_is_warmup_enabled', Boolean);
f('hphp_thread_set_warmup_enabled', NULL);

f('hphp_get_thread_id', Int64);
