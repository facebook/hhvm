<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

abstract class HasShapeConst {
  abstract const type TConfig as shape('host' => string, 'port' => int, ...);

  public function parse(this::TConfig $cfg): void {
    shape('host' => $host, 'port' => $port, ...) = $cfg;
    hh_expect_equivalent<string>($host);
    hh_expect_equivalent<int>($port);
  }
}

abstract class HasTupleConst {
  abstract const type TPair as (int, string);

  public function unpack(this::TPair $p): void {
    tuple($a, $b) = $p;
    hh_expect_equivalent<int>($a);
    hh_expect_equivalent<string>($b);
  }
}
