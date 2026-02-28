<?hh
<<__ConsistentConstruct>>
class D<T> {}

final class C<T3 as D<TAux>> {
  public function __construct(private classname<T3> $kls3) {}

  public function main(): int {
    $kls = $this->kls3;
    return new $kls();
  }
}
