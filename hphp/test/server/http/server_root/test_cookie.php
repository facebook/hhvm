<?hh
<<__EntryPoint>>
function test_cookie_entrypoint() :mixed{
setcookie(\HH\global_get('_GET')['cookie_name'],
          \HH\global_get('_GET')['cookie_value'],
          0,
          (string)(\HH\global_get('_GET')['cookie_path'] ?? ""),
          (string)(\HH\global_get('_GET')['cookie_domain'] ?? ""));
var_dump(\HH\global_get('_GET')['cookie_name']);
}
