<?php
var_dump(ini_get("hhvm.allow_hhas"));
var_dump(ini_get("hhvm.jit_timer"));
var_dump(ini_get("hhvm.simulate_arm"));
var_dump(ini_get("hhvm.jit_type_prediction"));
// This should return false
var_dump(ini_get("hhvm.this_should_not_work"));
var_dump(ini_get("hhvm.gdb_sync_chunks"));
// Not in our custom ini file for this test,
// but should be bound to a default value anyway.
var_dump(ini_get("hhvm.hot_func_count"));
