<?hh

<<__SupportDynamicType>>
class C {}

class D {}

function test1(D $d): void {
  // D is not SDT so should fail
  HH\FIXME\UNSAFE_CAST<D, dynamic>($d);
}

function test2(dynamic $d): void {
  // D is not SDT so should fail
  HH\FIXME\UNSAFE_CAST<dynamic, D>($d);
}

function test3(dynamic $d): vec<C> {
  // Type is vec<~C> so should fail
  return vec[HH\FIXME\UNSAFE_CAST<dynamic, C>($d)];
}
