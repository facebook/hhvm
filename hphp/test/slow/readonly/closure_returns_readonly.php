<?hh
final class MyBar {
  public int $x = 0;
}

final class MyFoo {
  public function __construct(private (function(): readonly MyBar) $getter) {}

  public function getBar(): MyBar {
    $f = $this->getter;
    $z = $f();
    return $z;
  }
}

function mutate(readonly MyBar $ro_bar): void {
  // launder it via MyFoo
  $foo = new MyFoo((): readonly ==> $ro_bar);
  $mut_bar = $foo->getBar();
  $mut_bar->x = 99;
}

<<__EntryPoint>>
function mymain(): void {
  $bar = readonly new MyBar();
  mutate($bar);
}
