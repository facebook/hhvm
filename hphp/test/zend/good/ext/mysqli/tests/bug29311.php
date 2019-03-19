<?php
	require_once("connect.inc");

	/* class 1 calls parent constructor */
	class mysql1 extends mysqli {
		function __construct() {

			parent::__construct(ZendGoodExtMysqliTestsConnectInc::$host,
                          ZendGoodExtMysqliTestsConnectInc::$user,
                          ZendGoodExtMysqliTestsConnectInc::$passwd,
                          ZendGoodExtMysqliTestsConnectInc::$db,
                          ZendGoodExtMysqliTestsConnectInc::$port,
                          ZendGoodExtMysqliTestsConnectInc::$socket);
		}
	}

	/* class 2 has an own constructor */
	class mysql2 extends mysqli {

		function __construct() {

			$this->connect(ZendGoodExtMysqliTestsConnectInc::$host,
                     ZendGoodExtMysqliTestsConnectInc::$user,
                     ZendGoodExtMysqliTestsConnectInc::$passwd,
                     ZendGoodExtMysqliTestsConnectInc::$db,
                     ZendGoodExtMysqliTestsConnectInc::$port,
                     ZendGoodExtMysqliTestsConnectInc::$socket);
		}
	}

	/* class 3 has no constructor */
	class mysql3 extends mysqli {

	}

	$foo[0] = new mysql1();
	$foo[1] = new mysql2();
	$foo[2] = new mysql3(ZendGoodExtMysqliTestsConnectInc::$host,
                       ZendGoodExtMysqliTestsConnectInc::$user,
                       ZendGoodExtMysqliTestsConnectInc::$passwd,
                       ZendGoodExtMysqliTestsConnectInc::$db,
                       ZendGoodExtMysqliTestsConnectInc::$port,
                       ZendGoodExtMysqliTestsConnectInc::$socket);


	for ($i=0; $i < 3; $i++) {
		if (($result = $foo[$i]->query("SELECT DATABASE()"))) {
			$row = $result->fetch_row();
			if ($row[0] != $db)
				printf("%d: %s\n", $i, $row[0]);
			$result->close();
		}
		$foo[$i]->close();
	}
	print "done!";
