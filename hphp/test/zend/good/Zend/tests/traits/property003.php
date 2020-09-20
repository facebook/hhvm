<?hh

<<__EntryPoint>>
function entrypoint_property003(): void {
  error_reporting(E_ALL);

  require_once(__DIR__.'/property003.traits.inc');

  echo "PRE-CLASS-GUARD\n";

  require_once(__DIR__.'/property003.class.inc');

  echo "POST-CLASS-GUARD\n";

  $t = new TraitsTest;
  $t->hello = "foo";
}
