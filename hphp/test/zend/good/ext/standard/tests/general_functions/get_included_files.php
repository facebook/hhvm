<?hh
<<__EntryPoint>>
function entrypoint_get_included_files(): void {
  /* Prototype: array get_included_files  ( void  )
   * Description: Returns an array with the names of included or required files

  */

  echo "*** Testing get_included_files()\n";

  echo "\n-- List included files at start --\n";
  var_dump(get_included_files());

  include(dirname(__FILE__)."/get_included_files_inc1.inc");
  inc1();
  echo "\n-- List included files atfter including inc1 -\n";
  var_dump(get_included_files());

  include(dirname(__FILE__)."/get_included_files_inc2.inc");
  inc2();
  echo "\n-- List included files atfter including inc2 which will include inc3 which includes inc1 --\n";
  var_dump(get_included_files());

  echo "\n-- Error cases --\n";
  try { var_dump(get_included_files(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "===DONE===\n";
}
