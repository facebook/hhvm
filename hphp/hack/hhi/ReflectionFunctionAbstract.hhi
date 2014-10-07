<?hh // decl

abstract class ReflectionFunctionAbstract implements Reflector {
    public $name = '';
    final private function __clone() {}
    public function __toString() {}
    public function inNamespace() {}
    public function isClosure() {}
    public function isDeprecated() {}
    public function isInternal() {}
    public function isUserDefined() {}
    public function getClosureThis() {}
    public function getClosureScopeClass() {}
    public function getDocComment() {}
    public function getEndLine() {}
    public function getExtension() {}
    public function getExtensionName() {}
    public function getFileName() {}
    public function getName() {}
    public function getNamespaceName() {}
    public function getNumberOfParameters() {}
    public function getNumberOfRequiredParameters() {}
    public function getParameters() {}
    public function getShortName() {}
    public function getStartLine() {}
    public function getStaticVariables() {}
    public function returnsReference() {}
}