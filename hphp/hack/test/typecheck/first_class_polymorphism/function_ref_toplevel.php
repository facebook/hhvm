<?hh
<<file: __EnableUnstableFeatures('function_references')>>
function top_id<T>(T $x): T {
  return $x;
}
function top_mono(int $x): int {
  return $x;
}
class Mock<Tfun> {
  public function __construct(private HH\FunctionRef<Tfun> $fun) {}
}
function testit(): void {
  $f = top_id<>;
  hh_show($f);
  $mock = new Mock($f);
  hh_show($mock);
  $g = top_mono<>;
  hh_show($g);
  $mockg = new Mock($g);
  hh_show($mockg);
}
