<?hh

<<__EntryPoint>>
function test_status_entrypoint() :mixed{
$uptime = HH\server_uptime();
var_dump($uptime);

$stopping = HH\server_is_stopping();
var_dump($stopping);

$prepared = HH\server_is_prepared_to_stop();
var_dump($prepared);

$health = HH\server_health_level();
var_dump($health);
}
