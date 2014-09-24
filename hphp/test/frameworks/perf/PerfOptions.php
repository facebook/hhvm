<?hh

final class PerfOptions {
  public bool $help;
  public bool $verbose;

  public ?string $php5;
  public ?string $hhvm;

  public string $siege;
  public string $nginx;

  public bool $wordpress;
  public bool $toys;
  public bool $sugarcrm;

  public bool $skipSanityCheck;
  public bool $skipVersionChecks;

  public bool $notBenchmarking;

  public function __construct() {
    $o = getopt(
      '',
      [
        'help',
        'verbose',
        'php5:', 'hhvm:',
        'siege:', 'nginx:',
        'toys', 'wordpress', 'sugarcrm-login-page',
        'i-am-not-benchmarking',
        'skip-sanity-check',
        'skip-version-checks',
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

    $this->skipSanityCheck = array_key_exists('skip-sanity-check', $o);
    $this->skipVersionChecks = array_key_exists('skip-version-checks', $o);
    $this->notBenchmarking = array_key_exists('i-am-not-benchmarking', $o);
  }
}
