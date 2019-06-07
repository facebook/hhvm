<?hh

class MyReflectionFunction extends ReflectionFunctionAbstract {
  public function __toString() {
    return "MyReflectionFunction";
  }
}


<<__EntryPoint>>
function main_extend_reflection_function_abstract() {
echo new MyReflectionFunction;
}
