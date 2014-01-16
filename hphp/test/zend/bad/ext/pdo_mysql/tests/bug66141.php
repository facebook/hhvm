<?php
include __DIR__ . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();

$input = 'Something\', 1 as one, 2 as two  FROM dual; -- f';

$quotedInput0 = $db->quote($input);

$db->query('set session sql_mode="NO_BACKSLASH_ESCAPES"');

// injection text from some user input

$quotedInput1 = $db->quote($input);

$db->query('something that throws an exception');

$quotedInput2 = $db->quote($input);

var_dump($quotedInput0);
var_dump($quotedInput1);
var_dump($quotedInput2);
?>
done