<?hh
/* Prototype  : proto bool is_subclass_of(object object, string class_name)
 * Description: Returns true if the object has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class B {
    public function inherit_pub() :mixed{}
    protected function inherit_prot() :mixed{}
    private function inherit_priv() :mixed{}
    static public function inherit_static_pub() :mixed{}
    static protected function inherit_static_prot() :mixed{}
    static private function inherit_static_priv() :mixed{}
}

class C extends B {
    public function pub() :mixed{}
    protected function prot() :mixed{}
    private function priv() :mixed{}
    static public function static_pub() :mixed{}
    static protected function static_prot() :mixed{}
    static private function static_priv() :mixed{}
}

<<__EntryPoint>> function method_exists_basic_001(): void {
$methods = vec[
    'inherit_pub', 'inherit_prot', 'inherit_priv',
    'inherit_static_pub', 'inherit_static_prot', 'inherit_static_priv',
    'pub', 'prot', 'priv',
    'static_pub', 'static_prot', 'static_priv',
    'non_existent'];

echo "\n ---(Using string class name)---\n";
foreach ($methods as $method) {
  echo "Does C::$method exist? ";
  var_dump(method_exists("C", $method));
}

echo "\n ---(Using object)---\n";
$myC = new C;
foreach ($methods as $method) {
  echo "Does C::$method exist? ";
  var_dump(method_exists($myC, $method));
}

echo "Done";
}
