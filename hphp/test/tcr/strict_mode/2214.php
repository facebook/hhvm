<?hh
function f() {
  $x = new Foo<Bar>();
  $x = new Foo<Blah<Bar>>();
  $y = new Foo<array,int>();
  $z = new Foo<Blah<array>,Blah<int>>();
  yo<int>();
  yo<Blah<int>>();
  yo<string,:my:xhp:class>();
  yo<Blah<string>,Blah<:my:xhp:class>>();
  $x->baz<int>();
  $x->baz<Blah<int>>();
  $y->baz<string,:my:xhp:class>();
  $y->baz<Blah<string>,Blah<:my:xhp:class>>();
  Foo<Bar>::biz();
  Foo<Blah<Bar>>::biz();
  Foo<Bar>::biz<string>();
  Foo<Blah<Bar>>::biz<Blah<string>>();
  Foo<Bar>::biz<bool,:my:xhp:class>();
  Foo<Blah<Bar>>::biz<Blah<bool>,Blah<:my:xhp:class>>();
  Foo<array,int>::biz();
  Foo<Blah<array>,Blah<int>>::biz();
  Foo<array,int>::biz<string>();
  Foo<Blah<array>,Blah<int>>::biz<Blah<string>>();
  Foo<array,int>::biz<bool,:my:xhp:class>();
  Foo<Blah<array>,Blah<int>>::biz<Blah<bool>,Blah<:my:xhp:class>>();
  var_dump(Foo<Bar>::SOME_CONST);
  var_dump(Foo<Blah<Bar>>::SOME_CONST);
  var_dump(Foo<array,int>::SOME_CONST);
  var_dump(Foo<Blah<array>,Blah<int>>::SOME_CONST);
  var_dump(Foo<Bar>::$staticProp);
  var_dump(Foo<Blah<Bar>>::$staticProp);
  var_dump(Foo<array,int>::$staticProp);
  var_dump(Foo<Blah<array>,Blah<int>>::$staticProp);
}
echo "Done\n";
