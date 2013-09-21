<?php
require(dirname(__FILE__). DIRECTORY_SEPARATOR . 'config.inc');
class PDO2 extends PDO {
	protected $transLevel;
}

class PDO3 extends PDO {
	protected $tomato;
}


class ModelA {
	public function __construct($h) {
		var_dump($h);
		if ($h) {
			$this->db = new PDO2(PDO_MYSQL_TEST_DSN, PDO_MYSQL_TEST_USER, PDO_MYSQL_TEST_PASS, array(PDO::ATTR_PERSISTENT => true));
		} else {
			$this->db = new PDO3(PDO_MYSQL_TEST_DSN, PDO_MYSQL_TEST_USER, PDO_MYSQL_TEST_PASS, array(PDO::ATTR_PERSISTENT => true));
		}
		$this->db->query('SELECT 1')->fetchAll();
	}
}

$a = new ModelA(true);
$b = new ModelA(false);

var_dump($a);
var_dump($b);