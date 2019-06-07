<?hh

function test() {
  if (!function_exists('apache_note')) {
    return false;
  }
  return "YES";
}


<<__EntryPoint>>
function main_apache_enabled() {
var_dump(test());
}
