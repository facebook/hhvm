<?hh


function id($x) { return $x; }

class c {}

<<__EntryPoint>> function main(): void {
  $name = 'varname';

  $x = $undef['foo'];

  try { $x = $GLOBALS[$name]['foo']; } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { $x = $GLOBALS[id($name)]['foo']; } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
