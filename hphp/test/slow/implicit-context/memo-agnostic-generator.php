<?hh

function genA(): AsyncGenerator<mixed,mixed,void>{
  echo 'genA:' . (string)TestContext::getContext() . "\n";
  yield 1;
  echo 'genA:' . (string)TestContext::getContext() . "\n";
  yield 2;
  echo 'genA:' . (string)TestContext::getContext() . "\n";
  yield 3;
}

function genB(): AsyncGenerator<mixed,mixed,void>{
  echo 'genB:' . (string)TestContext::getContext() . "\n";
  yield 1;
  echo 'genB:' . (string)TestContext::getContext() . "\n";
  yield 2;
}

<<__EntryPoint>>
function main(): mixed{
  include 'memo-agnostic.inc';

  TestContext::start(1, () ==> {
    $a = genA();
    $a->next(); // 1
    TestContext::start(2, () ==> {
      $b = genB();
      $b->next(); // 2
      $a->next(); // 2
      $b->next(); // 2
    });
    $a->next(); // 1
  });
}
