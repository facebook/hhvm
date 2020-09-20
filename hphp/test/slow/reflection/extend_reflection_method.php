<?hh

class Mock_MethodReflection extends ReflectionMethod {
  public function getAttributeRecursive($name) {}
  public function inNamespace() {}
  public function getNamespaceName() {}
  public function getShortName() {}
}


<<__EntryPoint>>
function main_extend_reflection_method() {
echo "Success.\n";
}
