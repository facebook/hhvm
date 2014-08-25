<?php
// These are set in our custom ini file for this test
var_dump(ini_get("hhvm.allow_hhas"));
var_dump(ini_get("hhvm.jit_timer"));
var_dump(ini_get("hhvm.simulate_arm"));
var_dump(ini_get("hhvm.jit_type_prediction"));
var_dump(ini_get("hhvm.gdb_sync_chunks"));

// These are not in our custom ini file for this test,
// but should be bound to a default value anyway
var_dump(ini_get("hhvm.hot_func_count"));
var_dump(ini_get("hhvm.jit_a_hot_size"));
var_dump(ini_get("hhvm.jit_always_interp_one"));
var_dump(ini_get("hhvm.log.use_log_file"));
var_dump(ini_get("hhvm.server.type"));
var_dump(ini_get("hhvm.server.port"));
var_dump(ini_get("hhvm.log.use_log_file"));
var_dump(ini_get("hhvm.log.file"));
var_dump(ini_get("hhvm.enable_zend_compat"));

// Throw some bad apples in there. They should
// all return false
var_dump(ini_get("hhvm.this_should_not_work"));
var_dump(ini_get("hhvm.jit_ahot_size"));
var_dump(ini_get("hhvm.ext_zend_compat"));
