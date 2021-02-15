<?hh

final class :a:post extends XHPTest {}

type alias = :a:post;

function test<reify T>(): void {
  $foo = <a:post>foo</a:post>;

  // Should error
  $bar = <notfound:post>bar</notfound:post>;

  // Does not allow generics
  $bar = <T>bar</T>;

  // Does not allow typedefs
  $bar = <alias>bar</alias>;
}
