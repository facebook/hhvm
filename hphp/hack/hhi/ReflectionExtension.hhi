<?hh // decl

class ReflectionExtension implements Reflector {
    public $name = '';
    final private function __clone() {}
    public static function export($name, $return = false) {}
    public function __construct($name) {}
    public function __toString() {}
    public function getName() {}
    public function getVersion() {}
    public function getFunctions() {}
    public function getConstants() {}
    public function getINIEntries() {}
    public function getClasses() {}
    public function getClassNames() {}
    public function getDependencies() {}
    public function info() {}
    public function isPersistent() {}
    public function isTemporary() {}
}

class ReflectionZendExtension implements Reflector {
    public $name = '';
    final private function __clone() {}
    public static function export($name, $return = null) {}
    public function __construct($name) {}
    public function __toString() {}
    public function getName() {}
    public function getVersion() {}
    public function getAuthor() {}
    public function getURL() {}
    public function getCopyright() {}
}
