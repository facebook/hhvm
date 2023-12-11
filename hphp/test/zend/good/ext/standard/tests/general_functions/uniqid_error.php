<?hh
/* Prototype  : string uniqid  ([ string $prefix= ""  [, bool $more_entropy= false  ]] )
 * Description: Gets a prefixed unique identifier based on the current time in microseconds.
 * Source code: ext/standard/uniqid.c */
class class1{}
<<__EntryPoint>> function main(): void {
echo "*** Testing uniqid() : error conditions ***\n";
echo "\n-- Testing uniqid() function with more than expected no. of arguments --\n";
$prefix = null;
$more_entropy = false;
$extra_arg = false;
try { var_dump(uniqid($prefix, $more_entropy, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing uniqid() function with invalid values for \$prefix --\n";
$obj = new class1();
$res = fopen(__FILE__, "r");
$array = vec[1,2,3];


try { uniqid($array, false); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { uniqid($res, false); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { uniqid($obj, false); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

fclose($res);

echo "===DONE===\n";
}
