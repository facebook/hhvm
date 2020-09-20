<?hh

function aux() {
  $x = IntContext::getContext();
  if ($x > 10) return;
  var_dump($x);
  IntContext::start($x+1, fun('aux'));
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  IntContext::start(0, fun('aux'));
}
