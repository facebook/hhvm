<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

class C { }

<<__SupportDynamicType>>
class D extends C { }

<<__SupportDynamicType>>
class A { }

function expectNonNull(nonnull $nn):void { }

function e(supportdyn<nonnull> $sd): void {
  expectNonNull($sd);
}

function expr(supportdyn<mixed> $sd): void {
  $sd === 1;
}

function stmt(\HH\supportdyn<mixed> $sd): void {
  if ($sd) {}
  switch ($sd) {
    case 42:
      break;
  }
}

function test1(D $d, C $c):void {
  e($d);
}

function test2(supportdyn<mixed> $sd):dynamic {
  return $sd upcast dynamic;
}

function test3(supportdyn<mixed> $sd):?supportdyn<nonnull> {
  return $sd;
}

function expectTrav<T as supportdyn<nonnull>>(Traversable<?T> $_):void { }

function test4(vec<supportdyn<mixed>> $v):void {
  expectTrav($v);
}

function voidfun():void { }

function callvoid():?supportdynamic {
  return voidfun();
}

function returnunion(bool $b):supportdynamic {
  return $b ? new A() : new D();
}
