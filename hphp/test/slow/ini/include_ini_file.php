<?hh

<<__EntryPoint>>
function main_include_ini_file() {
var_dump(ini_get("hhvm.server.port"));
var_dump(ini_get("hhvm.server.connection_limit"));
var_dump(ini_get("hhvm.server.thread_count"));
}
