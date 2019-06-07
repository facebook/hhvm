<?hh

class MyReflectionMethod extends ReflectionMethod
{
    public function toString()
    {
        return "custom toString";
    }
}


<<__EntryPoint>>
function main_reflection_method_override_tostring() {
$method = new MyReflectionMethod('MyReflectionMethod', 'toString');
echo $method->toString();
}
