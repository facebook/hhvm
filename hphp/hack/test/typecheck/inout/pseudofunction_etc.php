<?hh

function hgoldstein(inout int $x, dynamic $v): void {
  invariant(inout $x, 'foo');
  HH\FIXME\UNSAFE_CAST<dynamic, dynamic>(inout $x);
  unset(inout $x);
  $d = dict[]
  unset(inout $d['k']);
  unset(inout $v?->foo);
  isset(inout $x);
}

class HgoldsteinParent {
  public function __construct(inout int $_): void {}
}

class HgoldsteinChild extends HgoldsteinParent {
  public function __construct(int $x): void {
    parent::__construct(inout $x);
  }
}
