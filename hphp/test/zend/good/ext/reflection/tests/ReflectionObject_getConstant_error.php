<?hh
class C {
    const myConst = 1;
}
<<__EntryPoint>> function main(): void {
$rc = new ReflectionObject(new C);
try { var_dump($rc->getConstant()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($rc->getConstant("myConst", "myConst")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
