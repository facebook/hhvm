<?hh
 /*
  Helper for simple tests to check return-value. Usage:

  $tests = <<<TESTS
   expected_return_value === expression
   2                     === 1+1
   4                     === 2*2
   FALSE                 === @ fopen('non_existent_file')
TESTS;
  include( 'tests/quicktester.inc' );

  Expect: OK

  Remember to NOT put a trailing ; after a line!

 */
function quicktester($left_fun, $right_fun, $operator = '===') {
	error_reporting(E_ALL);

	$left  = $left_fun();
	$right = $right_fun();
	switch ($operator) {
		case '===': // exact match
			$result = $left === $right;
			break;
		case '~==': // may differ after 12th significant number
			if (   !is_float($left ) && !is_int($left )
				|| !is_float($right) && !is_int($right)) {
				$result = FALSE;
				break;
			}
			$result = abs(($left-$right) / $left) < 1e-12;
			break;
		default:
			echo "WARNING: unknown operator in '$test' (2)\n";
			exit;
	}

	if (!$result) {
		echo "\nAssert failed:\n";
		echo "Left:  ";var_dump($left );
		echo "Right: ";var_dump($right);
	} else {
		echo "OK\n";
	}
}
