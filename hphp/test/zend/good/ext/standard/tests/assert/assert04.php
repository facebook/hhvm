<?hh
<<__EntryPoint>> function main(): void {
/* Assert not active */
assert_options(ASSERT_ACTIVE, 0);
assert(1);

/* Wrong parameter count in assert */
assert_options(ASSERT_ACTIVE, 1);
try { assert(2, "failure", 3); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Wrong parameter count in assert_options */
try { assert_options(ASSERT_ACTIVE, 0, 2); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Wrong parameter name in assert_options */
$test="ASSERT_FRED";
try { assert_options($test, 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Assert false */
assert(0);


/* Assert false and bail*/
assert_options(ASSERT_BAIL, 1);
assert(0);

echo "not reached\n";
}
