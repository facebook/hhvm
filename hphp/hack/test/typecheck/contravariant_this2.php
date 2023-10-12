<?hh // strict

final class C<+T> {
  public function __construct(private T $v) {}
  public function put1(this $v): void {}
}
