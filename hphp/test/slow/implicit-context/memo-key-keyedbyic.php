<?hh

<<__Memoize(#KeyedByIC)>>
function memo($a, $b)[zoned] :mixed{
  echo "Memokey: ".HH\ImplicitContext\_Private\get_implicit_context_memo_key()."\n";
}

function f4()[zoned] :mixed{
  echo "in f4\n";
  memo(1, 2);
}

function f3()[zoned] :mixed{
  echo "in f3\n";
  memo(1, 2);
  ClassContext2::start(new A(0), f4<>);
}
function f2()[zoned] :mixed{
  echo "in f2\n";
  memo(1, 2);
  ClassContext::start(new A(0), f3<>);
}
function f1()[zoned] :mixed{
  echo "in f1\n";
  memo(1, 2);
  ClassContext2::start(new A(0), f2<>);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f1<>);
}
