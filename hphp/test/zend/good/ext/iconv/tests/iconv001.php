<?hh
<<__EntryPoint>>
function main_entry(): void {
  /* include('test.inc'); */
  echo "iconv extension is available\n";
  $test = "\xe6\xf8\xe5";
  var_dump("ISO-8859-1: $test");
  var_dump("UTF-8: ".iconv( "ISO-8859-1", "UTF-8", $test ) );
}
