<?hh

<<__EntryPoint>>
function large_response_entrypoint() :mixed{
$len = 1024 * 1024;
$data = str_repeat('x', $len);
var_dump($data);
}
