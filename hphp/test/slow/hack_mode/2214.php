<?hh
function f() :mixed{
  $x = new Foo<Bar>();
  $x = new Foo<Blah<Bar>>();
  $y = new Foo<AnyArray,int>();
  $z = new Foo<Blah<AnyArray>,Blah<int>>();
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
  Foo<AnyArray,int>::biz();
  Foo<Blah<AnyArray>,Blah<int>>::biz();
  Foo<AnyArray,int>::biz<string>();
  Foo<Blah<AnyArray>,Blah<int>>::biz<Blah<string>>();
  Foo<AnyArray,int>::biz<bool,:my:xhp:class>();
  Foo<Blah<AnyArray>,Blah<int>>::biz<Blah<bool>,Blah<:my:xhp:class>>();
  var_dump(Foo<Bar>::SOME_CONST);
  var_dump(Foo<Blah<Bar>>::SOME_CONST);
  var_dump(Foo<AnyArray,int>::SOME_CONST);
  var_dump(Foo<Blah<AnyArray>,Blah<int>>::SOME_CONST);
  var_dump(Foo<Bar>::$staticProp);
  var_dump(Foo<Blah<Bar>>::$staticProp);
  var_dump(Foo<AnyArray,int>::$staticProp);
  var_dump(Foo<Blah<AnyArray>,Blah<int>>::$staticProp);
}

<<__EntryPoint>>
function main_2214() :mixed{
echo "Done\n";
}
