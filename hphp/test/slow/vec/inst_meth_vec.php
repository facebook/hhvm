<?hh

class C {
  <<__DynamicallyCallable>> public function mth() {
    return 1;
  }
}


<<__EntryPoint>>
function main_inst_meth_vec() {
$c = new C;
$m = inst_meth($c, 'mth');
var_dump($m());
}
