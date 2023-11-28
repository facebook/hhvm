<?hh

<<__SupportDynamicType>>
class C {}

<<__SupportDynamicType>>
interface I { }

function f(C $c): ~I {
  return HH\FIXME\UNSAFE_CAST<C, I>($c); // expect 0 errors
}
