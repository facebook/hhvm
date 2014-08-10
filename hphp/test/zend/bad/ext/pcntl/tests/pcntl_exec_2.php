<?php
if (getenv("PCNTL_EXEC_TEST_IS_CHILD")) {
	var_dump((binary)getenv("FOO"));
	exit;
}
echo "ok\n";
pcntl_exec(getenv("TEST_PHP_EXECUTABLE"), array('-n', __FILE__), array(
	b"PCNTL_EXEC_TEST_IS_CHILD" => b"1", 
	b"FOO" => b"BAR",
	1 => b"long")
);

echo "nok\n";
?>