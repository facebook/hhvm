<?hh

<<__Memoize>>
function memo_fn($a, $b)[leak_safe]: void {
  echo "args: $a, $b \n";
}

<<__EntryPoint>>
function main(): mixed{
  include 'implicit.inc';
  echo memo_fn(1, 2);
  ClassContext::start(new A(0), () ==> {
    echo memo_fn(1, 2);
  });
}
