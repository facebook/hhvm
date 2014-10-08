<?hh // decl

class ReflectionProperty implements Reflector {
    const IS_STATIC = 1;
    const IS_PUBLIC = 256;
    const IS_PROTECTED = 512;
    const IS_PRIVATE = 1024;
    public $name = '';
    public $class = '';
    final private function __clone() {}
    public static function export($class, $name, $return = null) {}
    public function __construct($class, $name) {}
    public function __toString() {}
    public function getName() {}
    public function getValue($object = null) {}
    public function setValue($object, $value = null) {}
    public function isPublic() {}
    public function isPrivate() {}
    public function isProtected() {}
    public function isStatic() {}
    public function isDefault() {}
    public function getModifiers() {}
    public function getDeclaringClass() {}
    public function getDocComment() {}
    public function setAccessible($accessible) {}
    public function getTypeText() {}
}