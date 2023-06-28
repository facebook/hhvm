<?hh

// These are set in our custom ini file for this test
<<__EntryPoint>>
function main_ini_binding() :mixed{
var_dump(ini_get("hhvm.allow_hhas"));
var_dump(ini_get("hhvm.jit_timer"));
var_dump(ini_get("hhvm.gdb_sync_chunks"));
var_dump(ini_get("hhvm.server.apc.ttl_limit"));
var_dump(ini_get("hhvm.sandbox.pattern"));

// These are not in our custom ini file for this test,
// but should be bound to a default value
var_dump(ini_get("hhvm.jit_always_interp_one"));
var_dump(ini_get("hhvm.log.use_log_file"));
var_dump(ini_get("hhvm.server.type"));
var_dump(ini_get("hhvm.server.port"));
var_dump(ini_get("hhvm.log.use_log_file"));
var_dump(ini_get("hhvm.log.file"));
var_dump(ini_get("hhvm.mysql.connect_timeout"));

// Throw some bad apples in there. They should
// all return false
var_dump(ini_get("hhvm.this_should_not_work"));
var_dump(ini_get("hhvm.jit_ahot_size"));
}
