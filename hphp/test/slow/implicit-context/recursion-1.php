<?hh

function aux()[zoned] :mixed{
  $x = IntContext::getContext();
  if ($x > 10) return;
  var_dump($x);
  IntContext::start($x+1, aux<>);
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  IntContext::start(0, aux<>);
}
