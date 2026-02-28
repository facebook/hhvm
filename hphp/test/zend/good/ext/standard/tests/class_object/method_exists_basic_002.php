<?hh
/* Prototype  : proto bool is_subclass_of(object object, string class_name)
 * Description: Returns true if the object has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function method_exists_basic_002(): void {
echo " ---(Internal classes, using string class name)---\n";
echo "Does Exception::getMessage exist? ";
var_dump(method_exists("Exception", "getMessage"));
echo "Does stdClass::nonexistent exist? ";
var_dump(method_exists("stdClass", "nonexistent"));

echo "\n ---(Internal classes, using class instance)---\n";
echo "Does Exception::getMessage exist? ";
var_dump(method_exists(new Exception, "getMessage"));
echo "Does stdClass::nonexistent exist? ";
var_dump(method_exists(new stdClass, "nonexistent"));

echo "Done";
}
