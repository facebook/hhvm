<?hh

class C {
  public function m(inout dict<string,int> $i): void {}
}

function main1(C $c): void {
  $d = dict['a' => 0];
  $c->m(inout $d);
}

function f(inout dict<string,int> $i): void {}

function main2(C $c): void {
  $d = dict['a' => 0];
  f(inout $d);
}
