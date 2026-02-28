<?hh

// taking references
class C2 {
  public function __invoke(inout $a0) :mixed{
    var_dump($a0);
    $__lval_tmp_0 = $a0;
    $a0++;
    return $__lval_tmp_0;
  }
}
<<__EntryPoint>> function main(): void {
$x = 0;
$c = new C2;
$c(inout $x);
var_dump($x);
 // $x = 1
}
