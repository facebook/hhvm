<?hh // strict

<<Foo(1,2,3), Bar>>
type T1 = int;

<<SingleAttribute>>
type T2 = ?string;

type Serialized_contra<-T> = string;
type Serialized_co<+T> = string;
