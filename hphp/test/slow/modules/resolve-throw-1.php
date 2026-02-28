<?hh


module b;

function pp_exn(Exception $e) :mixed{
  return $e->getMessage() . " @ " . $e->getFile() . ":" . $e->getLine() . "\n";
}

<<__EntryPoint>>
function main() :mixed{
  include "resolve-1.inc1";
  include "resolve-1.inc";

  try { bar1(); } catch (Exception $e) { echo pp_exn($e); }
  $f = bar2();
  try { $f(); } catch (Exception $e) { echo pp_exn($e); }
  $f = bar3();
  try { $f(); } catch (Exception $e) { echo pp_exn($e); }
  try { bar4(); } catch (Exception $e) { echo pp_exn($e); }
  $f = bar5();
  try { $f(); } catch (Exception $e) { echo pp_exn($e); }
  $f = bar6();
  try { $f(); } catch (Exception $e) { echo pp_exn($e); }
  try { bar7(); } catch (Exception $e) { echo pp_exn($e); }
  $f = bar8();
  try { $f(); } catch (Exception $e) { echo pp_exn($e); }

  $c = cbar1();
  try { new $c; } catch (Exception $e) { echo pp_exn($e); }
  $c = cbar2();
  try { $f(); } catch (Exception $e) { echo pp_exn($e); }
  try { cbar3(); } catch (Exception $e) { echo pp_exn($e); }
}
