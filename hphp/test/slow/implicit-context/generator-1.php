<?hh

function genA() {
  echo 'genA:' . (string)IntContext::getContext() . "\n";
  yield 1;
  echo 'genA:' . (string)IntContext::getContext() . "\n";
  yield 2;
  echo 'genA:' . (string)IntContext::getContext() . "\n";
  yield 3;
}

function genB() {
  echo 'genB:' . (string)IntContext::getContext() . "\n";
  yield 1;
  echo 'genB:' . (string)IntContext::getContext() . "\n";
  yield 2;
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  IntContext::start(1, () ==> {
    $a = genA();
    $a->next(); // 1
    IntContext::start(2, () ==> {
      $b = genB();
      $b->next(); // 2
      $a->next(); // 2
      $b->next(); // 2
    });
    $a->next(); // 1
  });
}
