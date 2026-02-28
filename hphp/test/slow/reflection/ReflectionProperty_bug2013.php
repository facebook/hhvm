<?hh
class foo
{
    public $bar;
}


<<__EntryPoint>>
function main_reflection_property_bug2013() :mixed{
error_reporting(-1);

$foo  = new foo;
$obj  = new ReflectionObject($foo);
$prop = $obj->getProperty('bar');

var_dump($prop->getValue($foo));
unset($foo->bar);
var_dump($prop->getValue($foo));
}
