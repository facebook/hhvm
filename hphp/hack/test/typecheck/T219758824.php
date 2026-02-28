<?hh

final class C<TBar> {
  public function __construct(private classname<TBar> $kls) {}
  public function main(): void {
    $kls = $this->kls;
    new $kls($undefined); // no type error
  }
}
