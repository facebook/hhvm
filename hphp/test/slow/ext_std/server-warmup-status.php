<?hh


<<__EntryPoint>>
function main_server_warmup_status() :mixed{
var_dump(function_exists('HH\server_warmup_status'));
var_dump(function_exists('HH\server_warmup_status_monotonic'));
}
