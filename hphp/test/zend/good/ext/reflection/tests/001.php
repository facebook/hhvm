<?hh

class ReflectionClassEx extends ReflectionClass
{
    public $bla;

    function getMethodNames()
:mixed    {
        $res = vec[];
        foreach($this->getMethods() as $m)
        {
            $res[] = $m->class . '::' . $m->name;
        }
        return $res;
    }
}
<<__EntryPoint>> function main(): void {
$r = new ReflectionClassEx('ReflectionClassEx');

$exp = varray [
  'UMLClass::__clone',
  'UMLClass::export',
  'UMLClass::__construct',
  'UMLClass::__toString',
  'UMLClass::getName',
  'UMLClass::isInternal',
  'UMLClass::isUserDefined',
  'UMLClass::isInstantiable',
  'UMLClass::getFileName',
  'UMLClass::getStartLine',
  'UMLClass::getEndLine',
  'UMLClass::getDocComment',
  'UMLClass::getConstructor',
  'UMLClass::getMethod',
  'UMLClass::getMethods',
  'UMLClass::getProperty',
  'UMLClass::getProperties',
  'UMLClass::getConstants',
  'UMLClass::getConstant',
  'UMLClass::getInterfaces',
  'UMLClass::isInterface',
  'UMLClass::isAbstract',
  'UMLClass::isFinal',
  'UMLClass::getModifiers',
  'UMLClass::isInstance',
  'UMLClass::newInstance',
  'UMLClass::getParentClass',
  'UMLClass::isSubclassOf',
  'UMLClass::getStaticProperties',
  'UMLClass::getDefaultProperties',
  'UMLClass::isIterateable',
  'UMLClass::implementsInterface',
  'UMLClass::getExtension',
  'UMLClass::getExtensionName'];

$miss = vec[];

$res = $r->getMethodNames();

foreach($exp as $m)
{
    if (!in_array($m, $exp))
    {
        $miss[] = $m;
    }
}

var_dump($miss);

$props = array_keys(get_class_vars('ReflectionClassEx'));
sort(inout $props);
var_dump($props);
var_dump($r->name);
echo "===DONE===\n";
}
