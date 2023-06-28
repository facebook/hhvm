<?hh
class privateCtorOld {
    private function privateCtorOld() :mixed{}
    public static function reflectionObjectFactory() :mixed{
        return new ReflectionObject(new self);
    }
}
<<__EntryPoint>> function main(): void {
$reflectionObject =  privateCtorOld::reflectionObjectFactory();
try { var_dump($reflectionObject->isInstantiable('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($reflectionObject->isInstantiable(0, null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
