<?hh
<<__EntryPoint>> function main(): void {
$item = darray ["one" => 1, "two" => 2, "THREE" => 3, "FOUR" => "four"];

/* use 'case' argument other than CASE_LOWER & CASE_UPPER */
try { var_dump(array_change_key_case($item, "CASE_UPPER"));  } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(array_change_key_case($item, 5)); 

/* when keys are different in terms of only case */
/* should return one value key pair with key being in lowercase */
var_dump( array_change_key_case( dict["ONE" => 1, "one" => 3, "One" => 4] ) ); 
try { var_dump( array_change_key_case( dict["ONE" => 1, "one" => 6, "One" => 5], "CASE_UPPER" ) );  } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* should return one value key pair with key being in uppercase */
var_dump( array_change_key_case( dict["ONE" => 1, "one" => 2, "One" => 3], CASE_UPPER ) ); 
var_dump( array_change_key_case( dict["ONE" => 1, "one" => 1, "One" => 2], 5 ) ); 

echo "end\n";
}
