<?php
/* Assert not active */
assert_options(ASSERT_ACTIVE, 0);
assert(1);
 
 
/* Wrong parameter count in assert */
assert_options(ASSERT_ACTIVE, 1);
assert(2, "failure", 3);
 
/* Wrong parameter count in assert_options */
assert_options(ASSERT_ACTIVE, 0, 2);
 
/* Wrong parameter name in assert_options */
$test="ASSERT_FRED";
assert_options($test, 1);
 
/* Assert false */
assert(0);
 
 
/* Assert false and bail*/
assert_options(ASSERT_BAIL, 1);
assert(0);
 
echo "not reached\n";
 
?>