<?hh

type SM = supportdyn<mixed>;

<<__SupportDynamicType>>
class SS {}

<<__SupportDynamicType>>
class N<Ta as SM> { }

function makeN<Ta as SM>(string $s): ~N<Ta> { return new N(); }

<<__SupportDynamicType>>
function foo<T as SM>(N<T> $key, int $_): void {}

<<__SupportDynamicType>>
function bar(N<SS> $key): void {}

function testit(): void {
  // If we write makeN<string> then all is fine
  $r = makeN('a');
  foo($r, 2);
  bar($r);
}
