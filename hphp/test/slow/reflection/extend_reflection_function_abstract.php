<?hh

class MyReflectionFunction extends ReflectionFunctionAbstract {
  public function __toString()[] :mixed{
    return "MyReflectionFunction";
  }
}


<<__EntryPoint>>
function main_extend_reflection_function_abstract() :mixed{
echo new MyReflectionFunction;
}
