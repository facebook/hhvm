<?hh
/* Prototype  : array each(&array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Test behaviour of each() when passed:
 * 1. a two-dimensional array
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing each() : usage variations ***\n";

$arr = array ('zero',
              array(1, 2, 3),
              'one' => 'un',
              array('a', 'b', 'c')
              );

echo "\n-- Pass each() a two-dimensional array --\n";
for ($i = 1; $i < count($arr); $i++) {
    var_dump( each(inout $arr) );
}

echo "Done";
}
