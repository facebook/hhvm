<?hh

<<file: __EnableUnstableFeatures('case_types')>>

class Foo {}
type TInt = int;
case type TUnion = TInt | Foo;
type TAlias = TUnion;

function abra(TAlias $x): void {
  var_dump($x);
}

<<__EntryPoint>>
function main() {
  abra(12);
  abra(new Foo);
}
