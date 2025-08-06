<?hh
final class TestClass {
  private static function TestFunction(dict<string, mixed> $d): void {
    $e = $d['test'] as dict<_, _>;
    $f = $e['test'];
    hh_log_level("show", 2);
    hh_show($d);
    hh_show($e);
    hh_show($f);
  }
}
