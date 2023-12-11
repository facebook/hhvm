<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(preg_grep(1,vec[],3,4)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(preg_grep(1, 2)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(preg_grep('/+/', vec[]));

$array = dict[5 => 'a', 'x' => '1', 'xyz'=>'q6', 6 => 'h20'];

var_dump(preg_grep('@^[a-z]+@', $array));
var_dump(preg_grep('@^[a-z]+@', $array, PREG_GREP_INVERT));

ini_set('pcre.recursion_limit', 1);
$error = null;
var_dump(preg_grep_with_error('@^[a-z]+@', $array, inout $error));
var_dump($error === PREG_RECURSION_LIMIT_ERROR);
}
