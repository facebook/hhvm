<?hh

function test() :mixed{
  if (!function_exists('apache_note')) {
    return false;
  }
  return "YES";
}


<<__EntryPoint>>
function main_apache_enabled() :mixed{
var_dump(test());
}
