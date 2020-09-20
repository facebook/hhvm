<?hh // strict

class Foo implements HH\TypeAliasAttribute { public function __construct(int... $x) {} }
class Bar implements HH\TypeAliasAttribute {}
class SingleAttribute implements HH\TypeAliasAttribute {}

<<Foo(1,2,3), Bar>>
type T1 = int;

<<SingleAttribute>>
type T2 = ?string;

type Serialized_contra<-T> = string;
type Serialized_co<+T> = string;
