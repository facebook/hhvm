<?hh

function f(inout bool $a) :mixed{}
function g(inout bool $a, bool $not_inout) :mixed{}
function h(bool $a) :mixed{}

// Unnamed arguments get renamed in the runtime and are exposed to
// hack callers using reflection. Changing their name can affect
// hack code.
function i(bool $_, bool $_, bool $_) :mixed{}

<<__EntryPoint>>
function main() :mixed{
  $rf = new ReflectionFunction('f');
  $rg = new ReflectionFunction('g');
  $rh = new ReflectionFunction('h');
  $ri = new ReflectionFunction('i');
  var_dump($rf->getParameters());
  var_dump($rg->getParameters());
  var_dump($rh->getParameters());
  var_dump($ri->getParameters());
  $inout = $p ==> $p->isInOut();
  var_dump(array_map($inout, $rf->getParameters()));
  var_dump(array_map($inout, $rg->getParameters()));
  var_dump(array_map($inout, $rh->getParameters()));
  var_dump(array_map($inout, $ri->getParameters()));
  var_dump($rf->__toString());
  var_dump($rg->__toString());
  var_dump($rh->__toString());
  var_dump($ri->__toString());
}
