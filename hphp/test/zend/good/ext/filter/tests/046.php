<?hh

function test_validation($val, $msg) :mixed{
	$f = filter_var($val, FILTER_VALIDATE_INT);
	echo "$msg filtered: "; var_dump($f); // filtered value (or false)
	echo "$msg is_long: "; var_dump(is_long($f)); // test validation
	echo "$msg equal: "; var_dump(HH\Lib\Legacy_FIXME\eq($val, $f)); // test equality of result
}
<<__EntryPoint>>
function main_entry(): void {
  $max = sprintf("%d", PHP_INT_MAX);
  switch($max) {
  case "2147483647": /* 32-bit systems */
  	$min = "-2147483648";
  	$overflow = "2147483648";
  	$underflow = "-2147483649";
  	break;
  case "9223372036854775807": /* 64-bit systems */
  	$min = "-9223372036854775808";
  	$overflow = "9223372036854775808";
  	$underflow = "-9223372036854775809";
  	break;
  default:
  	exit("failed: unknown value for PHP_MAX_INT");
  	break;
  }

  // PHP_INT_MAX
  test_validation($max, "max");
  test_validation($overflow, "overflow");
  test_validation($min, "min");
  test_validation($underflow, "underflow");
}
