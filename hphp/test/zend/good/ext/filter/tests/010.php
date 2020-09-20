<?hh
<<__EntryPoint>> function main(): void {
var_dump(filter_var(varray[1,"1","", "-23234", "text", "asdf234asdfgs", varray[]], FILTER_VALIDATE_INT, FILTER_REQUIRE_ARRAY));
var_dump(filter_var(varray[1.2,"1.7","", "-23234.123", "text", "asdf234.2asdfgs", varray[]], FILTER_VALIDATE_FLOAT, FILTER_REQUIRE_ARRAY));
try { var_dump(filter_var(1, varray[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(filter_var(1, FILTER_SANITIZE_STRING, 1));
var_dump(filter_var(1, FILTER_SANITIZE_STRING, 0));
var_dump(filter_var(1, FILTER_SANITIZE_STRING, varray[]));
var_dump(filter_var(1, -1, varray[123]));
var_dump(filter_var(1, 0, varray[]));

echo "Done\n";
}
