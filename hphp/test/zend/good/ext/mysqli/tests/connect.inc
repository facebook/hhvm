<?php
	/*
	Default values are "localhost", "root",
	database "stest" and empty password.
	Change the MYSQL_TEST environment values
	if you want to use another configuration
	*/

	$driver    = new mysqli_driver;

  ZendGoodExtMysqliTestsConnectInc::$host      = getenv("MYSQL_TEST_HOST")     ? getenv("MYSQL_TEST_HOST") : "localhost";
  ZendGoodExtMysqliTestsConnectInc::$port      = getenv("MYSQL_TEST_PORT")     ? getenv("MYSQL_TEST_PORT") : 3306;
  ZendGoodExtMysqliTestsConnectInc::$user      = getenv("MYSQL_TEST_USER")     ? getenv("MYSQL_TEST_USER") : "root";
  ZendGoodExtMysqliTestsConnectInc::$passwd    = getenv("MYSQL_TEST_PASSWD")   ? getenv("MYSQL_TEST_PASSWD") : "";
  ZendGoodExtMysqliTestsConnectInc::$db        = getenv("MYSQL_TEST_DB")       ? getenv("MYSQL_TEST_DB") : "test";
  ZendGoodExtMysqliTestsConnectInc::$engine    = getenv("MYSQL_TEST_ENGINE")   ? getenv("MYSQL_TEST_ENGINE") : "MyISAM";
  ZendGoodExtMysqliTestsConnectInc::$socket    = getenv("MYSQL_TEST_SOCKET")   ? getenv("MYSQL_TEST_SOCKET") : null;
  ZendGoodExtMysqliTestsConnectInc::$skip_on_connect_failure  = getenv("MYSQL_TEST_SKIP_CONNECT_FAILURE") ? getenv("MYSQL_TEST_SKIP_CONNECT_FAILURE") : true;
	ZendGoodExtMysqliTestsConnectInc::$connect_flags = getenv("MYSQL_TEST_CONNECT_FLAGS") ? (int)getenv("MYSQL_TEST_CONNECT_FLAGS") : 0;
	if (ZendGoodExtMysqliTestsConnectInc::$socket) {
		ini_set('mysqli.default_socket', ZendGoodExtMysqliTestsConnectInc::$socket);
	}

	/* Development setting: test experimal features and/or feature requests that never worked before? */
	$TEST_EXPERIMENTAL = (in_array(getenv("MYSQL_TEST_EXPERIMENTAL"), array(0, 1))) ?
				((1 == getenv("MYSQL_TEST_EXPERIMENTAL")) ? true : false) :
				false;

	$IS_MYSQLND = stristr(mysqli_get_client_info(), "mysqlnd");
	if (!$IS_MYSQLND) {
		$MYSQLND_VERSION = NULL;
	} else {
		/*
		The formatting of the version reported by mysqli_get_client_info()
		has changed significantly in the past. To get tests working properly
		with PHP 5.3.0 and up, we set everything that looks like prior to
		PHP 5.3.0 to version 5.0.4 = 5 * 10000 + 0 * 100 + 4 = 50004.
		PHP 5.3.0	reports mysqlnd 5.0.5 dev (= 5 * 10000 + 0 * 100 + 5 = 50005.
		*/
		if (preg_match('@Revision:\s+(\d+)\s*\$@ism', mysqli_get_client_info(), $matches)) {
			/* something prior to PHP 5.3.0 */
			$MYSQLND_VERSION = 50004;
		} else if (preg_match('@^mysqlnd (\d+)\.(\d+)\.(\d+).*@ism', mysqli_get_client_info(), $matches)) {
			/* formatting schema used by PHP 5.3.0 */
			$MYSQLND_VERSION = (int)$matches[1] * 10000 + (int)$matches[2] * 100 + (int)$matches[3];
		} else if (preg_match('@^mysqlnd/PHP 6.0.0-dev@ism', mysqli_get_client_info(), $matches)) {
			/*
				PHP 6.0 at the time of the first PHP 5.3.0 release.
				HEAD and 5.3 have been in sync when 5.3.0 was released.
				It is at least 5.0.5-dev.
			*/
			$MYSQLND_VERSION = 50005;
		} else {
			/* unknown */
			$MYSQLND_VERSION = -1;
		}

	}

  /**
   * Whenever possible, please use this wrapper to make testing ot MYSQLI_CLIENT_COMPRESS (and potentially SSL) possible
   *
   * @param enable_env_flags Enable setting of connection flags through 	env(MYSQL_TEST_CONNECT_FLAGS)?
   */
  function my_mysqli_connect($host, $user, $passwd, $db, $port, $socket, $enable_env_flags = true) {


    $flags = ($enable_env_flags) ? ZendGoodExtMysqliTestsConnectInc::$connect_flags : false;

    if ($flags !== false) {
      $link = mysqli_init();
      if (!mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket, $flags))
        $link = false;
    } else {
      $link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);
    }

    return $link;
  }

  /**
   * Whenever possible, please use this wrapper to make testing ot MYSQLI_CLIENT_COMPRESS (and potentially SSL) possible
   *
   * @param enable_env_flags Enable setting of connection flags through env(MYSQL_TEST_CONNECT_FLAGS)
   */
  function my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket, $flags = 0, $enable_env_flags = true) {


    if ($enable_env_flags)
      $flags & ZendGoodExtMysqliTestsConnectInc::$connect_flags;

    return mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket, $flags);
  }

  class my_mysqli extends mysqli {
    public function __construct($host, $user, $passwd, $db, $port, $socket, $enable_env_flags = true) {


      $flags = ($enable_env_flags) ? ZendGoodExtMysqliTestsConnectInc::$connect_flags : false;

      if ($flags !== false) {
        parent::init();
        $this->real_connect($host, $user, $passwd, $db, $port, $socket, $flags);
      } else {
        parent::__construct($host, $user, $passwd, $db, $port, $socket);
      }
    }
  }

  function have_innodb($link) {
    if (($res = $link->query("SHOW VARIABLES LIKE 'have_innodb'")) &&
      ($row = $res->fetch_row()) &&
      !empty($row)) {
      if ($row[1] == "DISABLED" || $row[1] == "NO") {
        return false;
      }
      return true;
    } else {
      /* MySQL 5.6.1+ */
      if ($res = $link->query("SHOW ENGINES")) {
        while ($row = $res->fetch_assoc()) {
          if (!isset($row['Engine']) || !isset($row['Support']))
            return false;

          if (('InnoDB' == $row['Engine']) &&
            (('YES' == $row['Support']) || ('DEFAULT' == $row['Support']))
          ) {
            return true;
          }
        }
        return false;
      } else {
        return false;
      }
    }
    return false;
  }

	function handle_catchable_fatal($errno, $error, $file, $line) {
		$errcodes = array();
			$constants = get_defined_constants();
			foreach ($constants as $name => $value) {
				if (substr($name, 0, 2) == "E_")
					$errcodes[$value] = $name;
			}
		printf("[%s] %s in %s on line %s\n",
			(isset($errcodes[$errno])) ? $errcodes[$errno] : $errno,
			 $error, $file, $line);

		return true;
	}

abstract final class ZendGoodExtMysqliTestsConnectInc {
  public static $host;
  public static $user;
  public static $passwd;
  public static $db;
  public static $port;
  public static $socket;
  public static $connect_flags;
  public static $engine;
  public static $skip_on_connect_failure;
}
