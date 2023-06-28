<?hh

function foo() :mixed{
  if (class_exists('bar')) {
    echo "yes\n";
  }
 else {
    echo "no\n";
  }
  include '1220.inc';
  if (class_exists('bar')) {
    echo "yes\n";
  }
 else {
    echo "no\n";
  }
}

<<__EntryPoint>>
function main_1220() :mixed{
foo();
}
