<?php

class Mock_MethodReflection extends ReflectionMethod {
  public function getAttributeRecursive($name) {}
  public function inNamespace() {}
  public function getNamespaceName() {}
  public function getShortName() {}
}

echo "Success.\n";
