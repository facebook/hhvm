<?hh

<<__SupportDynamicType>>
function getx<T as nonnull as supportdyn<mixed>>(T $value): ~classname<T> {
  throw new TypeAssertionException("");
}

<<__SupportDynamicType>>
  function map<Tv1 as supportdyn<mixed>, Tv2 as arraykey as supportdyn<mixed>>(
  Traversable<Tv1> $traversable,
  (function(Tv1): ~Tv2) $value_func,
) : keyset<Tv2> { return keyset[]; }


<<__SupportDynamicType>>
  function my_vec<Tv as supportdyn<mixed>>(Traversable<Tv> $arr)[]: ~vec<Tv> { return vec[]; }

<<__SupportDynamicType>>
class B {}

<<__SupportDynamicType>>
enum class C : ~B {
  B a = new B();
}

<<__NoAutoDynamic>>
function test() : ~keyset<classname<HH\MemberOf<C, B>>> {
  $c = C::getValues();
  $v = my_vec($c);
  $x = map($v,($item) ==> {
    $g = getx($item);
    return $g;
  });
  return $x;
}

<<__NoAutoDynamic>>
function test2() : ~keyset<classname<HH\MemberOf<C, B>>> {
  $v = vec(C::getValues());
  return map($v,($item) ==> getx($item));
}
