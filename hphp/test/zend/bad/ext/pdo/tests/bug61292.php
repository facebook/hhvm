<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';

class Database_SQL extends PDO
{
	function __construct()
	{
                $dsn = getenv('PDOTEST_DSN');
                $user = getenv('PDOTEST_USER');
                $pass = getenv('PDOTEST_PASS');

                if ($user === false) $user = NULL;
                if ($pass === false) $pass = NULL;
		$options = array(PDO::ATTR_PERSISTENT => TRUE);

		parent::__construct($dsn, $user, $pass, $options);
	}

	var $bar = array();

	public function foo()
	{
		var_dump($this->bar);
	}
}

(new Database_SQL)->foo();
?>