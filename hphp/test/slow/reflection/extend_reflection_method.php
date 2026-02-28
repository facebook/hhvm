<?hh

class Mock_MethodReflection extends ReflectionMethod {
  public function getAttributeRecursive($name) :mixed{}
  public function inNamespace() :mixed{}
  public function getNamespaceName() :mixed{}
  public function getShortName() :mixed{}
}


<<__EntryPoint>>
function main_extend_reflection_method() :mixed{
echo "Success.\n";
}
