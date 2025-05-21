<?hh

function aux()[zoned] :mixed{
  $c = ClassContext::getContext();
  $x = $c ? $c->getPayload() : 0;
  if ($x > 10) return;
  var_dump($x);
  ClassContext::start(new Base($x+1), aux<>);
  var_dump(ClassContext::getContext()->getPayload());
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  ClassContext::start(new Base(0), aux<>);
}
