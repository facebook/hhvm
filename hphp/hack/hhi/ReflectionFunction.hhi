<?hh // decl

class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {
    const IS_DEPRECATED = 262144;
    public $name = '';
    public function __construct($name) {}
    public static function export($name, $return = null) {}
    public function isDisabled() {}
    public function invoke($args = null) {}
    public function invokeArgs(array $args) {}
    public function getClosure() {}
}