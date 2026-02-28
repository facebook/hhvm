<?hh
class C {
    const myConst = 1;
}
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass("C");
echo "Check invalid params:\n";
try { var_dump($rc->getConstant()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($rc->getConstant("myConst", "myConst")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
