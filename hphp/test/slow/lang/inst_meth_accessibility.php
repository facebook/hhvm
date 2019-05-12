<?hh

class C {
  private function private_meth($s) {
    echo "private: $s\n";
  }
  public function caller($f, $a) {
    $f($a);
  }
  public function getCallable() {
    return inst_meth($this, 'private_meth');
  }
}
<<__EntryPoint>> function main(): void {
$c = new C();
$pub = $c->getCallable();
$c->caller($pub, 'created in C');

$pri = inst_meth($c, 'private_meth');
$c->caller($pri, 'created outside');
}
