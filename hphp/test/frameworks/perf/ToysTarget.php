<?hh

require_once('PerfTarget.php');

final class ToysTarget extends PerfTarget {
  public function getSourceRoot(): string {
    return __DIR__.'/toys';
  }
}
