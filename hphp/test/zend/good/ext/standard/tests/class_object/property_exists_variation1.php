<?hh
/* Prototype  : bool property_exists(mixed object_or_class, string property_name)
 * Description: Checks if the object or class has a property
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

<<__EntryPoint>> function main(): void {
HH\autoload_set_paths(
  dict[
    'class' => dict[
      'autotest' => 'AutoTest.inc',
    ],
  ],
  __DIR__.'/',
);

echo "*** Testing property_exists() : class auto loading ***\n";

echo "\ntesting property in autoloaded class\n";
var_dump(property_exists("AutoTest", "bob"));

echo "\ntesting __get magic method\n";
var_dump(property_exists("AutoTest", "foo"));

echo "===DONE===\n";
}
