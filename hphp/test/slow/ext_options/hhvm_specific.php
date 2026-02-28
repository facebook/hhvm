<?hh


<<__EntryPoint>>
function main_hhvm_specific() :mixed{
var_dump(ini_get('hhvm.stats.profiler_trace_expansion'));
var_dump(ini_get('hhvm.pid_file'));
}
