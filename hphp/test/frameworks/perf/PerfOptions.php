<?hh

final class PerfOptions {
  public bool $help;
  public bool $verbose;

  public ?string $php5;

  public ?string $hhvm;
  public string $hhvmExtraArguments;

  public string $siege;
  public string $nginx;

  public bool $wordpress;
  public bool $toys;
  public bool $sugarcrm;

  public bool $skipSanityCheck;
  public bool $skipVersionChecks;
  public bool $traceSubProcess;

  //
  // All times are given in seconds, stored in a double.
  // For PHP code, the usleep timer is used, so fractional seconds work fine.
  //
  // For times that go into configuration files for 3rd party software,
  // such as nginx, times may be truncated to the nearest integer value,
  // in order to accomodate inflexibility in the 3rd party software.
  //
  public double $delayNginxStartup;
  public double $delayPhpStartup;
  public double $delayProcessLaunch;  // secs to wait after start process
  public double $delayCheckHealth;    // secs to wait before hit /check-health

  //
  // Maximum wait times, as for example given to file_get_contents
  // or the configuration file for nginx.  These times may be truncated
  // to the nearest integral second to accomodate the specific server.
  //
  public double $maxdelayUnfreeze;
  public double $maxdelayAdminRequest;
  public double $maxdelayNginxKeepAlive;
  public double $maxdelayNginxFastCGI;

  public bool $daemonOutputToFile = false;
  public ?string $tempDir = null;

  public bool $notBenchmarking = false;

  public function __construct() {
    $o = getopt(
      '',
      [
        'help',

        'verbose',

        'php5:',
        'hhvm:',
        'siege:',
        'nginx:',

        'toys',
        'wordpress',
        'sugarcrm-login-page',
        'i-am-not-benchmarking',

        'hhvm-extra-arguments:',

        'skip-sanity-check',
        'skip-version-checks',
        'skip-database-install',
        'trace',

        'delay-nginx-startup:',
        'delay-php-startup:',
        'delay-process-launch:',
        'delay-check-health:',

        'max-delay-unfreeze:',
        'max-delay-admin-request:',
        'max-delay-nginx-fastcgi:',

        'daemon-files',  // daemon output goes to files in the temp directory
        'temp-dir:',  // temp directory to use; if absent one in /tmp is made
      ]
    );
    $this->help = array_key_exists('help', $o);
    $this->verbose = array_key_exists('verbose', $o);

    $this->php5 = hphp_array_idx($o, 'php5', null);
    $this->hhvm = hphp_array_idx($o, 'hhvm', null);

    $this->siege = hphp_array_idx($o, 'siege', 'siege');
    $this->nginx = hphp_array_idx($o, 'nginx', 'nginx');

    $this->wordpress = array_key_exists('wordpress', $o);
    $this->toys = array_key_exists('toys', $o);
    $this->sugarcrm = array_key_exists('sugarcrm-login-page', $o);
    $this->traceSubProcess = array_key_exists('trace', $o);

    $this->notBenchmarking = array_key_exists('i-am-not-benchmarking', $o);

    $this->setConvenienceFlag(
      $this->skipSanityCheck,
      'skip-sanity-check',
      $o,
    );
    $this->setConvenienceFlag(
      $this->skipVersionChecks,
      'skip-version-checks',
      $o,
    );
    $this->setConvenienceFlag(
      $this->skipDatabaseInstall,
      'skip-database-install',
      $o,
    );

    //
    // If any arguments below here are given, then the "standard
    // semantics" have changed, and any results are potentially not
    // consistent with the benchmark standards for HHVM. You can only
    // use these arguments if you also give the -i-am-not-benchmarking
    // argument too.
    //
    $given_args = "";
    $this->hhvmExtraArguments =
      $this->get_string_arg($o, 'hhvm-extra-arguments', '', &$given_args,);
    $this->delayNginxStartup =
      $this->get_double_arg($o, 'delay-nginx-startup', 0.0, &$given_args,);
    $this->delayPhpStartup =
      $this->get_double_arg($o, 'delay-php-startup', 0.0, &$given_args,);
    $this->delayProcessLaunch =
      $this->get_double_arg($o, 'delay-process-launch', 1.0, &$given_args,);
    $this->delayCheckHealth =
      $this->get_double_arg($o, 'delay-check-health', 1.0, &$given_args,);

    $this->maxdelayUnfreeze =
      $this->get_double_arg($o, 'max-delay-unfreeze', 60.0, &$given_args,);
    $this->maxdelayAdminRequest =
      $this->get_double_arg($o, 'max-delay-admin-request', 3.0, &$given_args,);

    $this->maxdelayNginxKeepAlive =
      $this->get_double_arg(
        $o,
        'max-delay-nginx-keep-alive',
        60.0,
        &$given_args,
      );
    $this->maxdelayNginxFastCGI =
      $this->get_double_arg($o, 'max-delay-nginx-fastcgi', 60.0, &$given_args,);

    $newvalue = array_key_exists('daemon-files', $o);
    if ($newvalue ^ $this->daemonOutputToFile) {
      $given_args .= ' --daemon-files';
    }
    $this->daemonOutputToFile = $newvalue;

    $this->tempDir =
      $this->get_string_arg($o, 'temp-dir', null, &$given_args,);

    if ($given_args !== "") {
      if (!$this->notBenchmarking) {
        fwrite(
          STDERR,
          "Unless you specifically use the argument %s ".
          "you may not use any of these arguments that you did: %s\n",
          '--i-am-not-benchmarking',
          $given_args,
        );
        exit(1);
      }
    }
  }

  private function setConvenienceFlag(bool &$value, string $name, array $opts) {
    $value = array_key_exists($name, $opts);
    if ($value) {
      invariant(
        $this->notBenchmarking,
        "You must specify --i-am-not-benchmarking if you specify --%s",
        $name
      );
    }
  }

  public static function get_string_arg(
    Array $options,
    String $index,
    ?string $def,
    string& $used_args,
  ) : ?string {
    if (array_key_exists($index, $options)) {
      $used_args .= ' --'.$index;
    }
    return hphp_array_idx($options, $index, $def);
  }

  public static function get_double_arg(
    Array $options,
    String $index,
    double $def,
    string& $used_args,
  ) : double {
    if (array_key_exists($index, $options)) {
      $used_args .= ' --'.$index;
    }
    return (double)hphp_array_idx($options, $index, $def);
  }

  //
  // Return the name of a file that should collect stdout
  // from daemon executions.  Returning null means that
  // the daemon stdout should go to a pipe attached to this process.
  //
  public function daemonOutputFileName(string $daemonName): ?string {
    if ($this->daemonOutputToFile) {
      return (($this->tempDir === null) ? '/tmp' : $this->tempDir)
        . '/' . $daemonName . '.out';
    } else {
      return null;
    }
  }
}
