<?hh


class wub {
  public static function wubwub($v) {
    return function() use ($v) { return $v; };
  }

  public static function wubwubwub() {
    return function() { return "I'm static"; };
  }
}
<<__EntryPoint>> function main(): void {
$fn = wub::wubwub('hello there');
var_dump($fn());
$fn = wub::wubwubwub();
var_dump($fn());
}
