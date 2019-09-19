<?hh

include "connect.inc";

<<__EntryPoint>>
function main_entry(): void {

  $tmp    = NULL;
  $link   = NULL;

  try { mysql_escape_string(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  var_dump(@mysql_escape_string("Am I a unicode string in PHP 6?"));
  var_dump(@mysql_escape_string('\\'));
  var_dump(@mysql_escape_string('"'));
  var_dump(@mysql_escape_string("'"));
  var_dump(@mysql_escape_string("\n"));
  var_dump(@mysql_escape_string("\r"));
  var_dump(@mysql_escape_string("foo" . chr(0) . "bar"));

  print "done!";
}
