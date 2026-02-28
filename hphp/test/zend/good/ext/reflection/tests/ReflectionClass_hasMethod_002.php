<?hh
class C {
    function f() :mixed{}
}
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass("C");
echo "Check invalid params:\n";
try { var_dump($rc->hasMethod()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($rc->hasMethod("f", "f")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
