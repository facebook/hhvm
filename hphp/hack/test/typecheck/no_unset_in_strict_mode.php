<?hh

function f(): void {
  $a = "hello";
  unset($a);
}

class C {
  public int $i = 0;
  public static dict<int, int> $s = dict[];

  public function f(): void {
    unset($this->i);
    unset(C::$s);
  }

  public function ok(dict<int, int> $d): void {
    unset($d[0]);
    unset(C::$s[0]);
  }
}
