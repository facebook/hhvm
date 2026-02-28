<?hh
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
<<__EntryPoint>> function get_object_vars_variation_002(): void {
$obj = new stdClass;
var_dump(get_object_vars($obj));

$obj->val = 'original.a';

$arr = get_object_vars($obj);
var_dump($arr);

$arr['val'] = 'changed.val';

var_dump($arr, $obj);
}
