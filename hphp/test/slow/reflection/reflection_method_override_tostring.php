<?hh

class MyReflectionMethod extends ReflectionMethod
{
    public function toString()
:mixed    {
        return "custom toString";
    }
}


<<__EntryPoint>>
function main_reflection_method_override_tostring() :mixed{
$method = new MyReflectionMethod('MyReflectionMethod', 'toString');
echo $method->toString();
}
