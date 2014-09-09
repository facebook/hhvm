<?hh

final class PerfOptions {
  public bool $help;

  public ?string $php5;
  public ?string $hhvm;

  public string $siege;
  public string $nginx;

  public bool $wordpress;
  public bool $toys;

  public bool $skipSanityCheck;
  public bool $skipVersionChecks;

  public function __construct() {
    $o = getopt(
      '',
      [
        'help',
        'php5:', 'hhvm:',
        'siege:', 'nginx:',
        'toys', 'wordpress',
        'skip-sanity-check',
        'skip-version-checks',
      ]
    );
    $this->help = array_key_exists('help', $o);

    $this->php5 = hphp_array_idx($o, 'php5', null);
    $this->hhvm = hphp_array_idx($o, 'hhvm', null);

    $this->siege = hphp_array_idx($o, 'siege', 'siege');
    $this->nginx = hphp_array_idx($o, 'nginx', 'nginx');

    $this->wordpress = array_key_exists('wordpress', $o);
    $this->toys = array_key_exists('toys', $o);

    $this->skipSanityCheck = array_key_exists('skip-sanity-check', $o);
    $this->skipVersionChecks = array_key_exists('skip-version-checks', $o);
  }
}
