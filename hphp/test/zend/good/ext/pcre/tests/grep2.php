<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(preg_grep(1,varray[],3,4)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(preg_grep(1, 2)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(preg_grep('/+/', varray[]));

$array = darray[5 => 'a', 'x' => '1', 'xyz'=>'q6', 6 => 'h20'];

var_dump(preg_grep('@^[a-z]+@', $array));
var_dump(preg_grep('@^[a-z]+@', $array, PREG_GREP_INVERT));

ini_set('pcre.recursion_limit', 1);
var_dump(preg_last_error() == PREG_NO_ERROR);
var_dump(preg_grep('@^[a-z]+@', $array));
var_dump(preg_last_error() == PREG_RECURSION_LIMIT_ERROR);
}
