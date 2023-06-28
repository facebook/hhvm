<?hh

function main($x, $y) :mixed{
  var_dump(class_exists($x, false));
  var_dump(class_exists($y, false));
  if (!class_exists('X')) {
    echo "Loading...\n";
    require_once('persistent.inc');
  }

  var_dump(new X, new Y);
}


<<__EntryPoint>>
function main_persistent1() :mixed{
main('X', 'Y');
}
