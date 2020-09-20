<?hh

final class Test {
  public static function filter<<<__Enforceable>> reify T>(vec<mixed> $list): vec<T> {
    $ret = vec[];
    foreach ($list as $elem) {
      if ($elem is T) {
        $ret[] = $elem;
      }
    }
    return $ret;
  }
}

<<__EntryPoint>>
function main(): void {
  $arr = vec[1, "foo", true, "bar", 12.0];

  $x = Test::filter<string>($arr);
  foreach($x as $el) {
    print($el."\n");
  }

  $y = Test::filter<string>;
  $z = $y($arr);
  foreach($z as $el) {
    print($el."\n");
  }
}
