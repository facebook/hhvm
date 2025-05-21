<?hh

function print_state(string $s, bool $b)[zoned]: void {
  echo $s.": ";
  echo $b ? 'True' : 'False';
  echo "\n";
}

function backdoored_fn(): void {
  echo "in backdoored_fn\n";
  print_state('ClassContext::exists', ClassContext::exists());
}

<<__Memoize(#KeyedByIC)>>
function memo($a, $b)[zoned]: mixed{
  echo "in keyedbyIC memo\n";
  print_state('ClassContext::exists', ClassContext::exists());
}

<<__Memoize>>
function memo2($a, $b): mixed{
  echo "in default memo\n";
  print_state('ClassContext::exists', ClassContext::exists());
}

function g()[zoned]: mixed{
  // backdoor should clear ic
  HH\Coeffects\backdoor(backdoored_fn<>);
}

function f()[zoned]: mixed{
  echo "in zoned...\n";
  print_state('ClassContext::exists', ClassContext::exists());
  memo(1, 3);
  g();
}

<<__EntryPoint>>
function main(): mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f<>);
  memo2(1, 4);
}
