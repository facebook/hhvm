<?hh

function test($val, $exp, $feature) :mixed{
  if ($val !== $exp) {
    echo $feature . " failed. Got:\n";
    var_dump($val);
    echo "But expected:\n";
    var_dump($exp);
  } else {
    echo $feature . " passed\n";
  }
}

<<__EntryPoint>>
function run() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    include '1463-1.inc';
  } else {
    include '1463-2.inc';
  }

  include '1463-child1.inc';
  include '1463-child2.inc';

  if (__hhvm_intrinsics\launder_value(true)) {
    include '1463-3.inc';
  } else {
    include '1463-4.inc';
  }

  $base = new base;
  test($base->baseVal, 'base', 'Base object member');
  test(base::$baseStatic, 'baseStat', 'Base static member');
  test(base::baseStatMeth(), 'Base static method', 'Base static method');
  $child1 = new child1;
  test($child1->baseVal, 'base', 'dRedec inherited property');
  test($child1->child1Val, 'child1', 'dRedec property');
  test($child1->testChildMeth(), 'I am child1', 'dRedec parent->virtual method');
  test($child1->childProp(), 'IamChild1', 'dRedec parent->child prop method');
  test(child1::child1StatMeth(), 'Child 1 static method', 'dRedec static method');
  test(child1::baseStatMeth(), 'Base static method', 'dRedec parent static method');
  test(child1::$baseStatic, 'baseStat', 'dRedec parent static prop');
  test(child1::$child1Static, 'child1Stat', 'dRedec static prop');
  test($child1->parentChildMeth(), 'I am base', 'dRedec parent method');
  $child2 = new child2;
  test($child2->baseVal, 'base', 'ddRedec grandparent property');
  test($child2->child1Val, 'child1', 'ddRedec parent property');
  test($child2->child2Val, 'child2', 'ddRedec property');
  test($child2->testChildMeth(), 'I am child2', 'ddRedec grandparent->virtual method');
  test($child2->testChildMeth2(), 'I am child2', 'ddRedec parent->virtual method');
  test($child2->childProp(), 'IamChild2', 'ddRedec grandparent->child prop method');
  test(child2::baseStatMeth(), 'Base static method', 'ddRedec grandparent static method');
  test(child2::child1StatMeth(), 'Child 1 static method', 'ddRedec parent static method');
  test(child2::child2StatMeth(), 'Child 2 static method', 'ddRedec static method');
  test(child2::$baseStatic, 'baseStat', 'ddRedec grandparent static prop');
  test(child2::$child1Static, 'child1Stat', 'ddRedec parent static prop');
  test(child2::$child2Static, 'child2Stat', 'ddRedec static prop');
  test($child2->parentChildMeth(), 'I am child1', 'ddRedec parent method');
  $child3 = new child3;
  test($child3->baseVal, 'base', 'RddRedec greatgrandparent property');
  test($child3->child1Val, 'child1', 'RddRedec grandparent property');
  test($child3->child2Val, 'child2', 'RddRedec parent property');
  test($child3->child3Val, 'child3', 'RddRedec property');
  test($child3->testChildMeth(), 'I am child3', 'RddRedec greatgrandparent->virtual method');
  test($child3->testChildMeth2(), 'I am child3', 'RddRedec grandparent->virtual method');
  test($child3->childProp(), 'IamChild3', 'RddRedec greatgrandparent->child prop method');
  test(child3::baseStatMeth(), 'Base static method', 'RddRedec greatgrandparent static method');
  test(child3::child1StatMeth(), 'Child 1 static method', 'RddRedec grandparent static method');
  test(child3::child2StatMeth(), 'Child 2 static method', 'RddRedec parent static method');
  test(child3::child3StatMeth(), 'Child 3 static method', 'RddRedec static method');
  test(child3::$baseStatic, 'baseStat', 'RddRedec greatgrandparent static prop');
  test(child3::$child1Static, 'child1Stat', 'RddRedec grandparent static prop');
  test(child3::$child2Static, 'child2Stat', 'RddRedec parent static prop');
  test(child3::$child3Static, 'child3Stat', 'RddRedec static prop');
  test($child3->parentChildMeth(), 'I am child2', 'RddRedec parent method');
}
