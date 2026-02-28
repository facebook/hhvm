<?hh
<<__EntryPoint>> function main(): void {
/* generate different failure conditions */
$int_var  = -19;
$item = dict["one" => 1, "two" => 2, "THREE" => 3, "FOUR" => "four"];
try { var_dump( array_change_key_case($int_var) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args less than expected
var_dump( array_change_key_case($int_var, CASE_UPPER) ); // invalid first argument
try { var_dump( array_change_key_case() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero argument
try { var_dump( array_change_key_case($item, $item["one"], "CASE_UPPER") );  } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // more than expected numbers

echo "end\n";
}
