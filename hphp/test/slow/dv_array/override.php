<?hh

abstract class ParentA { abstract function foo(AnyArray $x):mixed; }
class ChildA1 extends ParentA { function foo(varray $x) :mixed{} }
class ChildA2 extends ParentA { function foo(darray $x) :mixed{} }
class ChildA3 extends ParentA { function foo(varray_or_darray $x) :mixed{} }

abstract class ParentB { abstract function foo(varray $x):mixed; }
class ChildB1 extends ParentB { function foo(varray $x) :mixed{} }
class ChildB2 extends ParentB { function foo(darray $x) :mixed{} }
class ChildB3 extends ParentB { function foo(varray_or_darray $x) :mixed{} }
class ChildB4 extends ParentB { function foo(AnyArray $x) :mixed{} }

abstract class ParentC { abstract function foo(darray $x):mixed; }
class ChildC1 extends ParentC { function foo(varray $x) :mixed{} }
class ChildC2 extends ParentC { function foo(darray $x) :mixed{} }
class ChildC3 extends ParentC { function foo(varray_or_darray $x) :mixed{} }
class ChildC4 extends ParentC { function foo(AnyArray $x) :mixed{} }

abstract class ParentD { abstract function foo(varray_or_darray $x):mixed; }
class ChildD1 extends ParentD { function foo(varray $x) :mixed{} }
class ChildD2 extends ParentD { function foo(darray $x) :mixed{} }
class ChildD3 extends ParentD { function foo(varray_or_darray $x) :mixed{} }
class ChildD4 extends ParentD { function foo(AnyArray $x) :mixed{} }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
