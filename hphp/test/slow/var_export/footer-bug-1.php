<?hh

<<__EntryPoint>>
function main_footer_bug_1() {
$str = var_export(varray[Vector {new stdClass()}], true);
// HHVM has a bug with var_export() where the output sometimes contains
// unnecessary blank lines. We do this substitution here so that this
// test's expected output is not affected by this bug.
$str = strtr($str, darray["\n    \n" => "\n"]);
echo $str, "\nDone\n";
}
