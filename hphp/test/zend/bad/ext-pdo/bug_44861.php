<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'oci') {
	$from = 'FROM DUAL';
	$ob = '1';
} else {
	$from = '';
	$ob = 'r';
}

$query = "SELECT 'row1' AS r $from UNION SELECT 'row2' $from UNION SELECT 'row3' $from UNION SELECT 'row4' $from ORDER BY $ob";
$aParams = array(PDO::ATTR_CURSOR => PDO::CURSOR_SCROLL);

$res = $db->prepare($query, $aParams);
$res->execute();
var_dump($res->fetchColumn());
var_dump($res->fetchColumn());
var_dump($res->fetchColumn());
var_dump($res->fetchColumn());
var_dump($res->fetchColumn());

var_dump($res->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_ABS, 3));
var_dump($res->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_PRIOR));
var_dump($res->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_FIRST));
var_dump($res->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_LAST));
var_dump($res->fetch(PDO::FETCH_NUM, PDO::FETCH_ORI_REL, -1));

var_dump($res->fetchAll(PDO::FETCH_ASSOC));

// Test binding params via emulated prepared query
$res = $db->prepare("SELECT ? $from", $aParams);
$res->execute(array("it's working"));
var_dump($res->fetch(PDO::FETCH_NUM));


// Test bug #48188, trying to execute again
$res->execute(array("try again"));
var_dump($res->fetchColumn());
var_dump($res->fetchColumn());

?>