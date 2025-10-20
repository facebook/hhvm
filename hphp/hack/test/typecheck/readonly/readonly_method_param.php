<?hh
class C{
public function test_fun_ro(readonly Vector<int> $v): void {}
public function test_fun(Vector<int> $v): void {}
}

function test_fun2(): void {
  $c = new C();
  $v = readonly Vector {1};
  $c->test_fun_ro($v);
}

function test_fun3(): void {
  $c = new C();
  $v = readonly Vector {1};
  $c->test_fun($v); // Should error
}

function test_fun4(): void {
  $c = new C();
  $v = Vector {1};
  $c->test_fun_ro($v);
}
