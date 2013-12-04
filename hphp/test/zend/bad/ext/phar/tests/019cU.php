<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$file = b"<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

$files = array();
$files['a'] = 'a';
$files['b/a'] = 'b';
$files['b/c/d'] = 'c';
$files['bad/c'] = 'd';
include 'files/phar_test.inc';
include $fname;

function dump($phar, $base)
{
	var_dump($phar . $base);
	$dir = opendir($phar . $base);
	if ($base == '/')
	{
		$base = '';
	}
	while (false !== ($entry = readdir($dir))) {
		$entry = $base . '/' . $entry;
		var_dump($entry);
		var_dump(is_dir($phar . $entry));
		if (is_dir($phar . $entry))
		{
			dump($phar, $entry);
		}
		else
		{
			var_dump(file_get_contents($phar . $entry));
		}
	}
}

dump('phar://hio', '/');

?>
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>