<?hh

class C {
  public function mth() {
    return 1;
  }
}

$c = new C;
$m = inst_meth($c, 'mth');
var_dump($m());
