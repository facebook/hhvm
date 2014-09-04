<?hh

require_once('PerfTarget.php');

final class ToysTarget extends PerfTarget {
  public function getSanityCheckPath(): string {
    return '/fibonacci.php';
  }

  public function getSanityCheckString(): string {
    return 'int(10946)';
  }
  public function getSourceRoot(): string {
    return __DIR__.'/toys';
  }
}
