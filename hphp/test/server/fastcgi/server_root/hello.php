<?hh

<<__EntryPoint>>
function hello_entrypoint() {
  $testid = getenv('TESTID');
  echo "Hello, World!{$testid}";
}
