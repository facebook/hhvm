<?hh

function genA() :AsyncGenerator<mixed,mixed,void>{
  echo 'genA:' . (string)ClassContext::getContext()->getPayload() . "\n";
  yield 1;
  echo 'genA:' . (string)ClassContext::getContext()->getPayload() . "\n";
  yield 2;
  echo 'genA:' . (string)ClassContext::getContext()->getPayload() . "\n";
  yield 3;
}

function genB() :AsyncGenerator<mixed,mixed,void>{
  echo 'genB:' . (string)ClassContext::getContext()->getPayload() . "\n";
  yield 1;
  echo 'genB:' . (string)ClassContext::getContext()->getPayload() . "\n";
  yield 2;
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  ClassContext::start(new Base(1), () ==> {
    $a = genA();
    $a->next(); // 1
    ClassContext::start(new Base(2), () ==> {
      $b = genB();
      $b->next(); // 2
      $a->next(); // 2
      $b->next(); // 2
    });
    $a->next(); // 1
  });
}
