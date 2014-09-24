<?hh

require_once('PerfTarget.php');

final class SugarCRMTarget extends PerfTarget {
  public function __construct(
    private string $tempDir
  ) {
  }

  protected function getSanityCheckString(): string {
    return 'User Name:';
  }

  public function install(): void {
    shell_exec($this->safeCommand(Vector {
      'tar',
      '-C', $this->tempDir,
      '-zxf',
      __DIR__.'/sugarcrm/sugarcrm_dev-6.5.16.tar.gz',
    }));

    $root = 'http://'.gethostname().':'.PerfSettings::HttpPort();
    $conn = mysql_connect('127.0.0.1', 'sugarcrm', 'sugarcrm');
    $db_selected = mysql_select_db('sugarcrm', $conn);
    if ($conn === false || $db_selected === false) {
      $this->createMySQLDatabase();
      $this->install();
      return;
    };

    shell_exec(
      $this->safeCommand(Vector {
        'zcat',
        __DIR__.'/sugarcrm/dbdump.sql.gz'
      }).'|'.
      $this->safeCommand(Vector {
        'mysql',
        '-h', '127.0.0.1',
        'sugarcrm',
        '-u', 'sugarcrm',
        '-psugarcrm',
      })
    );

    copy(
      __DIR__.'/sugarcrm/config.php',
      $this->getSourceRoot().'/config.php',
    );
  }

  private function createMySQLDatabase(): void {
    fprintf(
      STDERR,
      '%s',
      "Can't connect to the sugarcrm MySQL database. You can manually fix ".
      "this, or enter your MySQL admin details.\nUsername: "
    );
    $username = trim(fgets(STDIN));
    if (!$username) {
      throw new Exception(
        'Invalid user - set up the sugarcrm database and user manually.'
      );
    }
    fprintf(STDERR, '%s', 'Password: ');
    $password = trim(fgets(STDIN));
    if (!$password) {
      throw new Exception(
        'Invalid password - set up the sugarcrm database and user manually.'
      );
    }
    $conn = mysql_connect('127.0.0.1', $username, $password);
    if ($conn === false) {
      throw new Exception(
        'Failed to connect: '.mysql_error()
      );
    }
    mysql_query('DROP DATABASE IF EXISTS sugarcrm', $conn);
    mysql_query('CREATE DATABASE sugarcrm', $conn);
    mysql_query(
      'GRANT ALL PRIVILEGES ON sugarcrm.* TO sugarcrm@"%" '.
      'IDENTIFIED BY "sugarcrm"',
      $conn
    );
  }

  public function getSourceRoot(): string {
    return $this->tempDir.'/sugarcrm_dev-6.5.16';
  }

  // See PerfTarget::ignorePath() for documentation
  public function ignorePath(string $path): bool {
    // Users don't actually request this
    if (strstr($path, 'wp-cron.php')) {
      return true;
    }
    return false;
  }
}
