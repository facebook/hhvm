<?hh
require_once('PHPEngineStats.php');

trait NoEngineStats implements PHPEngineStats {
  public function enableStats(): void {
  }

  public function collectStats(): Map<string, Map<string, num>> {
    return Map { };
  }
};
