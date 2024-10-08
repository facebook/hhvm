<?hh

interface Foo {}
interface Bar {}
interface Baz {}

case type CaseTypeWithInvariantGeneric<T> =
  | int where T as Foo
  | string where T super Bar
  | bool where T = Baz;

case type CaseTypeWithCovariantGeneric<+T> =
  | int where T as Foo // error
  | string where T super Bar
  | bool where T = Baz; // error

case type CaseTypeWithContravariantGeneric<-T> =
  | int where T as Foo
  | string where T super Bar // error
  | bool where T = Baz; // error
