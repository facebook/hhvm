<?hh
/* Prototype  : proto string ob_get_contents(void)
 * Description: Return the contents of the output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing ob_get_contents() : error cases ***\n";

try { var_dump(ob_get_contents("bob")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

ob_start();

try { var_dump(ob_get_contents("bob2",345)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
