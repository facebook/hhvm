//// file1.php
<?hh // strict

class C<T> {}
newtype B = C<mixed>;

//// file2.php
<?hh // strict

class A {}

type Saopt = shape(?'a' => ?A);
type Sa = shape('a' => A);
type Sbopt = shape(?'b' => ?B);
type Sb = shape('b' => B);


function f(bool $b, Saopt $xa, Sa $ya, Sbopt $xb, Sb $yb): void {
  if ($b) {
    $z = $xa;
  } else {
    $z = $ya;
  }
  hh_show($z);

  if ($b) {
    $z = $xb;
  } else {
    $z = $yb;
  }
  hh_show($z);
}
