<?hh

function main($z) :mixed{
  var_dump(function_exists($z, false));
  var_dump(function_exists($z, true));
  if (!function_exists('Z')) {
    echo "Loading...\n";
    require_once('persistent.inc');
  }

  var_dump(Z());
}


<<__EntryPoint>>
function main_persistent2() :mixed{
main('Z');
}
