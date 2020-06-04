<?hh

<<__EntryPoint>>
function entrypoint_property004(): void {
  error_reporting(E_ALL);

  require_once(__DIR__.'/property004.traits.inc');

  echo "PRE-CLASS-GUARD\n";

  require_once(__DIR__.'/property004.class.inc');

  $t = new TraitsTest;
}
