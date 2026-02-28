<?hh

class MyReflectionFunction extends ReflectionFunction
{
    public function toString()
:mixed    {
        return 'custom toString';
    }
}


<<__EntryPoint>>
function main_reflection_function_override_tostring() :mixed{
$function = new MyReflectionFunction('str_replace');
echo $function->toString();
}
