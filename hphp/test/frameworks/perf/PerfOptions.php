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
  public double $delayServerStabilize;
  public double $delayProcessLaunch;  // secs to wait after start process
  public double $delayCheckHealth;    // secs to wait before hit /check-health

  //
  // Maximum wait times, as for example given to file_get_contents
  // or the configuration file for nginx.  These times may be truncated
  // to the nearest second to accomodate the specific server.
  //
  public double $maxdelayUnfreeze;
  public double $maxdelayAdminRequest;
  public double $maxdelayNginxKeepAlive;
  public double $maxdelayNginxFastCGI;

  public bool $daemonOutputToFile;    // if sub process output goes to file
  public ?string $tempDir;

  public bool $notBenchmarking;

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
        'i-am-not-benchmarking',

        'hhvm-extra-arguments:',

        'skip-sanity-check',
        'skip-version-checks',
        'trace',

        'delay-nginx-startup:',
        'delay-php-startup:',
        'delay-server-stabilize:',
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
    $this->hhvmExtraArguments = hphp_array_idx($o, 'hhvm-extra-arguments', '');

    $this->wordpress = array_key_exists('wordpress', $o);
    $this->toys = array_key_exists('toys', $o);
    $this->traceSubProcess = array_key_exists('trace', $o);

    $this->skipSanityCheck = array_key_exists('skip-sanity-check', $o);
    $this->skipVersionChecks = array_key_exists('skip-version-checks', $o);
    $this->notBenchmarking = array_key_exists('i-am-not-benchmarking', $o);

    $this->delayNginxStartup = (double)hphp_array_idx($o, 'delay-nginx-startup', 0.0);
    $this->delayPhpStartup = (double)hphp_array_idx($o, 'delay-php-startup', 0.0);
    $this->delayServerStabilize = (double)hphp_array_idx($o, 'delay-server-stabilize', 30.0);
    $this->delayProcessLaunch = (double)hphp_array_idx($o, 'delay-process-launch', 1.0);
    $this->delayCheckHealth = (double)hphp_array_idx($o, 'delay-check-health', 1.0);

    $this->maxdelayUnfreeze = (double)hphp_array_idx($o, 'max-delay-unfreeze', 60.0);
    $this->maxdelayAdminRequest = (double)hphp_array_idx($o, 'max-delay-admin-request', 3.0);

    $this->maxdelayNginxKeepAlive =
      (double)hphp_array_idx($o, 'max-delay-nginx-keep-alive', 60);
    $this->maxdelayNginxFastCGI =
      (double)hphp_array_idx($o, 'max-delay-nginx-fastcgi', 60);

    $this->daemonOutputToFile = array_key_exists('daemon-files', $o);
    $this->tempDir = hphp_array_idx($o, 'temp-dir', null);
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
