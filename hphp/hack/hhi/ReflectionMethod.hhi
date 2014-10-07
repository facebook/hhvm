<?hh // decl

class ReflectionMethod extends ReflectionFunctionAbstract implements Reflector {
    const IS_STATIC = 1;
    const IS_PUBLIC = 256;
    const IS_PROTECTED = 512;
    const IS_PRIVATE = 1024;
    const IS_ABSTRACT = 2;
    const IS_FINAL = 4;
    public $name = '';
    public $class = '';
    public static function export($class, $name, $return = false) {}
    public function __construct($class, $name) {}
    public function isPublic() {}
    public function isPrivate() {}
    public function isProtected() {}
    public function isAbstract() {}
    public function isFinal() {}
    public function isStatic() {}
    public function isConstructor() {}
    public function isDestructor() {}
    public function getClosure($object) {}
    public function getModifiers() {}
    public function invoke($object, $parameter = null, $_ = null) {}
    public function invokeArgs($object, array $args) {}
    public function getDeclaringClass() {}
    public function getPrototype() {}
    public function setAccessible($accessible) {}
}