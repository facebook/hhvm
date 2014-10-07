<?hh // decl

class ReflectionParameter implements Reflector {
    public $name = '';
    final private function __clone() {}
    public static function export($function, $parameter, $return = null) {}
    public function __construct($function, $parameter) {}
    public function __toString() {}
    public function getName() {}
    public function isPassedByReference() {}
    public function canBePassedByValue() {}
    public function getDeclaringFunction() {}
    public function getDeclaringClass() {}
    public function getClass() {}
    public function isArray() {}
    public function isCallable() {}
    public function allowsNull() {}
    public function getPosition() {}
    public function isOptional() {}
    public function isDefaultValueAvailable() {}
    public function getDefaultValue() {}
    public function isDefaultValueConstant() {}
    public function getDefaultValueConstantName() {}
}