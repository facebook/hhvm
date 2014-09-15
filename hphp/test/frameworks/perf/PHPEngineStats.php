<?hh

interface PHPEngineStats {
  public function enableStats(): void;
  public function collectStats(): Map<string, Map<string, num>>;
};
