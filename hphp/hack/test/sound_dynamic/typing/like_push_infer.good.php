<?hh

<<__SupportDynamicType>>
  function my_vec<Tv as supportdyn<mixed>>(Traversable<Tv> $arr)[]: ~vec<Tv> { return vec[]; }

<<__SupportDynamicType>>
class B {}

<<__SupportDynamicType>>
enum class C : ~B {
  B a = new B();
}

<<__NoAutoDynamic>>
function test(~dict<string, HH\MemberOf<C, ~B>> $c) : ~vec<HH\MemberOf<C, B>> {
  $v = my_vec($c);
  return $v;
}
