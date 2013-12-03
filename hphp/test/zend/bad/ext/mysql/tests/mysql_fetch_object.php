<?php
include "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (!is_null($tmp = @mysql_fetch_object()))
	printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

if (false !== ($tmp = @mysql_fetch_object($link)))
	printf("[002] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');
if (!$res = mysql_query("SELECT id AS ID, label FROM test AS TEST ORDER BY id LIMIT 5", $link)) {
	printf("[003] [%d] %s\n", mysql_errno($link), mysql_error($link));
}

var_dump(mysql_fetch_object($res));

class mysql_fetch_object_test {

	public $a = null;
	public $b = null;

	public function toString() {
		var_dump($this);
	}
}

var_dump(mysql_fetch_object($res, 'mysql_fetch_object_test'));

class mysql_fetch_object_construct extends mysql_fetch_object_test {

	public function __construct($a, $b) {
		$this->a = $a;
		$this->b = $b;
	}

}

var_dump(mysql_fetch_object($res, 'mysql_fetch_object_construct', null));
var_dump(mysql_fetch_object($res, 'mysql_fetch_object_construct', array('a')));
var_dump(mysql_fetch_object($res, 'mysql_fetch_object_construct', array('a', 'b')));
var_dump(mysql_fetch_object($res, 'mysql_fetch_object_construct', array('a', 'b', 'c')));
var_dump(mysql_fetch_object($res, 'mysql_fetch_object_construct', "no array and not null"));
var_dump(mysql_fetch_object($res));
var_dump(mysql_fetch_object($res, 'mysql_fetch_object_construct', array('a', 'b')));

class mysql_fetch_object_private_construct {
	private function __construct($a, $b) {
		var_dump($a);
	}
}
var_dump(mysql_fetch_object($res, 'mysql_fetch_object_private_construct', array('a', 'b')));

mysql_free_result($res);

if (!$res = mysql_query("SELECT id AS ID, label FROM test AS TEST", $link)) {
	printf("[009] [%d] %s\n", mysql_errno($link), mysql_error($link));
}

mysql_free_result($res);

var_dump(mysql_fetch_object($res));

// Fatal error, script execution will end
var_dump(mysql_fetch_object($res, 'this_class_does_not_exist'));

mysql_close($link);
print "done!";
?>
<?php
require_once("clean_table.inc");
?>