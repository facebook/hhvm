<?hh
class privateCtorOld {
    private function privateCtorOld() {}
    public static function reflectionObjectFactory() {
        return new ReflectionObject(new self);
    }
}
<<__EntryPoint>> function main(): void {
$reflectionObject =  privateCtorOld::reflectionObjectFactory();
try { var_dump($reflectionObject->IsInstantiable('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($reflectionObject->IsInstantiable(0, null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
