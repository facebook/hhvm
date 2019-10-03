<?hh

function f(inout bool $a) {}
function g(inout bool $a, bool $not_inout) {}
function h(bool $a) {}

<<__EntryPoint>>
function main() {
  $rf = new ReflectionFunction('f');
  $rg = new ReflectionFunction('g');
  $rh = new ReflectionFunction('h');
  var_dump($rf->getParameters());
  var_dump($rg->getParameters());
  var_dump($rh->getParameters());
  $inout = $p ==> $p->isInOut();
  var_dump(array_map($inout, $rf->getParameters()));
  var_dump(array_map($inout, $rg->getParameters()));
  var_dump(array_map($inout, $rh->getParameters()));
  var_dump($rf->__toString());
  var_dump($rg->__toString());
  var_dump($rh->__toString());
}
