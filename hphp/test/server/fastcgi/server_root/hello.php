<?hh

<<__EntryPoint>>
function hello_entrypoint() :mixed{
  $testid = getenv('TESTID');
  echo "Hello, World!{$testid}";
}
