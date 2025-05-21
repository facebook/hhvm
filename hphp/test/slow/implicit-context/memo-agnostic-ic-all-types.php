<?hh

function f1()[leak_safe]: void {
  HH\Coeffects\fb\backdoor_from_leak_safe__DO_NOT_USE(
    ()[defaults] ==> var_dump(TestContext::getContext()),
    'Testing memoization',
  );
}

function f2(): void {
  echo "inside f2, calling f1 with a new baggage 7\n";
  TestContext::start(7, f1<>);
  echo "returned, now the baggage should be 6\n";
  var_dump(TestContext::getContext());
}

async function f3(): Awaitable<int> {
  return 15;
}

function f4(): int {
  return 20;
}

async function f5(): Awaitable<dict> {
  return dict['x' => 0, 'y' => 1, 'z' => 2];
}

function indirection(): void {
  var_dump(TestContext::getContext());
}

function f6(): void {
  indirection();
}

<<__Memoize>>
function memo_indirection(): void {
  var_dump(TestContext::getContext());
}

function f7(): void {
  echo "calling memo_indirection...\n";
  memo_indirection();
  echo "returned. fetching context again\n";
  var_dump(TestContext::getContext());
}

function f8(): void {
  $res = TestContext::getContext();
  echo "result: ";
  var_dump($res);
  echo "result, resolved: ";
  var_dump($res());
}

function f9(): void {
  $res = TestContext::getContext();
  echo "result: ";
  var_dump($res);
  echo "result, awaited: ";
  var_dump(\HH\Asio\join($res()));
}

<<__EntryPoint>>
function main(): void {
  include 'memo-agnostic.inc';

  $d = dict['a' => 0, 'b' => 1, 'c' => 2];
  echo "\n1. Set null\n";
  TestContext::start(null, f1<>);

  echo "\n2. Set 5\n";
  TestContext::start(5, f1<>);

  echo "\n3. Set String\n";
  TestContext::start("I don't have friends, I got family", f1<>);

  echo "\n4. Set Dict\n";
  TestContext::start($d, f1<>);

  echo "\n5. Set nested baggage\n";
  TestContext::start(6, f2<>);


  echo "\n6. Set lambda 10\n";
  TestContext::start(()[] ==> { return 10; }, f8<>);

  echo "\n7. Set async func result 15\n";
  TestContext::start(f3<>, f9<>);

  echo "\n8. Set func result 20\n";
  TestContext::start(f4<>, f8<>);

  echo "\n9. Set async func result Dict\n";
  TestContext::start(f5<>, f9<>);

  echo "\n10. Set 17. call via an indirection\n";
  TestContext::start(17, f6<>);

  echo "\n11. Set 18. call a memoized indirection via indirection\n";
  TestContext::start(10, f7<>);
}
