<?hh
class privateCtorOld {
    private function privateCtorOld() {}
}
<<__EntryPoint>> function main(): void {
$reflectionClass = new ReflectionClass("privateCtorOld");
try { var_dump($reflectionClass->IsInstantiable('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($reflectionClass->IsInstantiable(0, null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
