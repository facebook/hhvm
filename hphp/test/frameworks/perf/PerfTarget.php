<?hh

abstract class PerfTarget {
  public function install(): void { }

  abstract protected function getSanityCheckString(): string;
  abstract public function getSourceRoot(): string;

  protected function getSanityCheckPath(): string {
    return '/';
  }

  final public function sanityCheck(): void {
    $url = 'http://'.gethostname().':'.PerfSettings::HttpPort().
      $this->getSanityCheckPath();
    $content = file_get_contents($url);
    invariant(
      strstr($content, $this->getSanityCheckString()) !== false,
      'Failed to find string "'.$this->getSanityCheckString().'" in '.
      $url
    );
  }

  final public function getURLsFile(): string {
    return __DIR__.'/'.get_class($this).'.urls';
  }

  /*
   * Blacklist paths from HHVM internal statistics. Some frameworks
   * make a request to themselves (eg wordpress calls /wp-cron.php on every
   * page load). The CPU time/instructions taken by this page don't actually
   * affect RPS, so reporting them isn't useful.
   *
   * The time taken to trigger the request is still included in the wall time
   * for the relevant page (eg index.php).
   *
   * THIS DOES NOT AFFECT THE OVERALL RESULTS FROM SIEGE.
   */
  public function ignorePath(string $path): bool {
    return false;
  }

  /*
   * Given the data in the .sql might be old, there could be some /ridiculously/
   * expensive stuff to do on the first request - for example, wordpress will
   * make a request to rpc.pingomatic.com, and it'll upgrade itself.
   */
  public function needsUnfreeze(): bool {
    return false;
  }

  public function unfreeze(): void {
    invariant_violation(
      'If you override needsUnfreeze(), you must override unfreeze() too.'
    );
  }
}
