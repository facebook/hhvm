<?php

require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

$pdodb = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');


function testQuery($query) {
	global $pdodb;
	$stmt = $pdodb->prepare($query);
	
	if (!$stmt->execute(array("foo"))) {
		var_dump($stmt->errorInfo());
	} else{
		var_dump($stmt->fetch(PDO::FETCH_ASSOC));
	}
}

testQuery("/* ' */ select ? as f1 /* ' */");
testQuery("/* '-- */ select ? as f1 /* *' */");
testQuery("/* ' */ select ? as f1 --';");
testQuery("/* ' */ select ? as f1 -- 'a;");
testQuery("/*'**/ select ? as f1 /* ' */");
testQuery("/*'***/ select ? as f1 /* ' */");
testQuery("/*'**a ***b / ****
******
**/ select ? as f1 /* ' */");

?>