<?hh
// PDO test framework utilities

function updateEnv() {
	if (getenv('PDOTEST_DSN') === false) {
		$conf = PDOTest::get_config(dirname($_SERVER['PHP_SELF']).'/common.phpt');
		foreach($conf['ENV'] as $n=>$v) putenv("$n=$v");
	}
}

class PDOTest {
	// create an instance of the PDO driver, based on
	// the current environment
	static function factory($classname = 'PDO', $drop_test_tables = true) {
		$dsn = getenv('PDOTEST_DSN');
		$user = getenv('PDOTEST_USER');
		$pass = getenv('PDOTEST_PASS');
		$attr = getenv('PDOTEST_ATTR');
		if (is_string($attr) && strlen($attr)) {
			$attr = unserialize($attr);
		} else {
			$attr = null;
		}

		if ($user === false) $user = '';
		if ($pass === false) $pass = '';

		$db = new $classname($dsn, $user, $pass, $attr);

		if (!$db) {
			exit("Could not create PDO object (DSN=$dsn, user=$user)\n");
		}

		// clean up any crufty test tables we might have left behind
		// on a previous run
		$test_tables = vec[
			'test',
			'test2',
			'classtypes'
			];
		if ($drop_test_tables) {
			foreach ($test_tables as $table) {
				$db->exec("DROP TABLE $table");
			}
		}

		$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);
		$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
		$db->setAttribute(PDO::ATTR_STRINGIFY_FETCHES, true);
		return $db;
	}

	static function skip() {
		try {
			$db = PDOTest::factory();
		} catch (PDOException $e) {
			exit("skip " . $e->getMessage());
		}
	}

	static function test_factory($file) {
		$config = self::get_config($file);
		foreach ($config['ENV'] as $k => $v) {
			putenv("$k=$v");
		}
		return self::factory();
	}

	<<__Memoize>>
	static function get_config($file) {
		$data = file_get_contents($file);
		$data = preg_replace('/^.*--REDIRECTTEST--/s', '', $data);
		eval($data);
		return redirecttest();
	}
}
