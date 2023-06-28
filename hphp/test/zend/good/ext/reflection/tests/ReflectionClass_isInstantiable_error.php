<?hh
class privateCtorOld {
    private function privateCtorOld() :mixed{}
}
<<__EntryPoint>> function main(): void {
$reflectionClass = new ReflectionClass("privateCtorOld");
try { var_dump($reflectionClass->isInstantiable('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($reflectionClass->isInstantiable(0, null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
