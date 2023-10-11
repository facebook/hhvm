<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum E: string as string {
  AA = 'AA';
}

<<__SupportDynamicType>>
class C { }

<<__SupportDynamicType>>
function foo<T as supportdyn<mixed>>(classname<T> $class_name): void { }

function bar<T as supportdyn<mixed>>(T $x):void { }

<<__SupportDynamicType>>
function testit(): void {
  bar(new C());
  foo(C::class);
  bar(E::AA);
  foo(E::class);
}
