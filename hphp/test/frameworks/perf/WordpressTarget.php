<?hh

require_once('PerfTarget.php');

final class WordpressTarget extends PerfTarget {
  public function __construct(
    private PerfOptions $options,
    private string $tempDir,
  ) {
  }

  public function getSanityCheckString(): string {
    return 'Recent Comments';
  }

  public function install(): void {
    shell_exec($this->safeCommand(Vector {
      'tar',
      '-C', $this->tempDir,
      '-zxf',
      __DIR__.'/wordpress/wordpress-3.9.1.tar.gz',
    }));

    copy(
      __DIR__.'/wordpress/wp-config.php',
      $this->getSourceRoot().'/wp-config.php',
    );

    if ($this->options->skipDatabaseInstall) {
      return;
    }

    $root = 'http://'.gethostname().':'.PerfSettings::HttpPort();
    $conn = mysql_connect('127.0.0.1', 'wp_bench', 'wp_bench');
    $db_selected = mysql_select_db('wp_bench', $conn);
    if ($conn === false || $db_selected === false) {
      $this->createMySQLDatabase();
      $this->install();
      return;
    };

    shell_exec(
      $this->safeCommand(Vector {
        'zcat',
        __DIR__.'/wordpress/dbdump.sql.gz'
      }).'|'.
      $this->safeCommand(Vector {
        'mysql',
        '-h', '127.0.0.1',
        'wp_bench',
        '-u', 'wp_bench',
        '-pwp_bench',
      })
    );

    // Reconnect - the above takes a while, leading to
    // 'the MySQL server has gone away' if we re-use.
    $conn = mysql_connect('127.0.0.1', 'wp_bench', 'wp_bench');
    $db_selected = mysql_select_db('wp_bench', $conn);
    $result = mysql_query(
      'UPDATE wp_options '.
      "SET option_value='".mysql_real_escape_string($root)."' ".
      'WHERE option_name IN ("siteurl", "home")',
      $conn
    );
    if ($result !== true) {
      throw new Exception(mysql_error());
    }
    mysql_query(
      'DELETE FROM wp_options WHERE option_name = "admin_email"',
      $conn
    );
  }

  private function createMySQLDatabase(): void {
    fprintf(
      STDERR,
      '%s',
      "Can't connect to database ".
      "(mysql -h 127.0.0.1 -pwp_bench -u wp_bench wp_bench). This can be ".
      "fixed for you.\nMySQL admin user (probably 'root'): "
    );
    $username = trim(fgets(STDIN));
    if (!$username) {
      throw new Exception(
        'Invalid user - set up the wp_bench database and user manually.'
      );
    }
    fprintf(STDERR, '%s', 'MySQL admin password: ');
    $password = trim(fgets(STDIN));
    if (!$password) {
      throw new Exception(
        'Invalid password - set up the wp_bench database and user manually.'
      );
    }
    $conn = mysql_connect('127.0.0.1', $username, $password);
    if ($conn === false) {
      throw new Exception(
        'Failed to connect: '.mysql_error()
      );
    }
    mysql_query('DROP DATABASE IF EXISTS wp_bench', $conn);
    mysql_query('CREATE DATABASE wp_bench', $conn);
    /* In theory, either one of these works, with 127.0.0.1 being the minimal
     * one.
     * - do % so that if someone debugs with localhost, hostname, or ::1 (IPv6)
     *   it works as expectedd
     * - do 127.0.0.1 as well, just in case there's a pre-existing incompatible
     *   grant
     */
    mysql_query(
      'GRANT ALL PRIVILEGES ON wp_bench.* TO wp_bench@"%" '.
      'IDENTIFIED BY "wp_bench"',
      $conn
    );
    mysql_query(
      'GRANT ALL PRIVILEGES ON wp_bench.* TO wp_bench@127.0.0.1 '.
      'IDENTIFIED BY "wp_bench"',
      $conn
    );
  }

  public function getSourceRoot(): string {
    return $this->tempDir.'/wordpress';
  }

  // See PerfTarget::ignorePath() for documentation
  public function ignorePath(string $path): bool {
    // Users don't actually request this
    if (strstr($path, 'wp-cron.php')) {
      return true;
    }
    return false;
  }

  public function needsUnfreeze(): bool {
    return true;
  }

  // Contact rpc.pingomatic.com, upgrade to latest .z release, other periodic
  // tasks
  public function unfreeze(): void {
    // Need internet access or wordpress will keep on retrying this stuff
    if (!file_get_contents('http://www.example.com')) {
      throw new Exception('Wordpress requires internet access');
    }
    // Does basic bookeeping...
    $this->unfreezeRequest();
    // Does more involved stuff like upgrading wordpress...
    $this->unfreezeRequest();
    // Let's just be paranoid and do it again.
    $this->unfreezeRequest();
  }

  private function unfreezeRequest(): void {
    $ctx = stream_context_create(
      ['http' => ['timeout' => 120]]
    );
    $data = file_get_contents(
      'http://'.gethostname().':'.PerfSettings::HttpPort().'/',
      /* include path = */ false,
      $ctx
    );
    invariant($data !== false, 'Failed to unfreeze');
  }
}
