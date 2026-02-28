<?hh

<<__ConsistentConstruct>>
class E {}

final class C<T2 as E> {
  public function __construct(private classname<T2> $kls2) {}

  public function main(): int {
    $kls = $this->kls2;
    return new $kls();
  }
}
