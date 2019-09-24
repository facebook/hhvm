<?hh

abstract class ParentA { abstract function foo(array $x); }
class ChildA1 extends ParentA { function foo(varray $x) {} }
class ChildA2 extends ParentA { function foo(darray $x) {} }
class ChildA3 extends ParentA { function foo(varray_or_darray $x) {} }

abstract class ParentB { abstract function foo(varray $x); }
class ChildB1 extends ParentB { function foo(varray $x) {} }
class ChildB2 extends ParentB { function foo(darray $x) {} }
class ChildB3 extends ParentB { function foo(varray_or_darray $x) {} }
class ChildB4 extends ParentB { function foo(array $x) {} }

abstract class ParentC { abstract function foo(darray $x); }
class ChildC1 extends ParentC { function foo(varray $x) {} }
class ChildC2 extends ParentC { function foo(darray $x) {} }
class ChildC3 extends ParentC { function foo(varray_or_darray $x) {} }
class ChildC4 extends ParentC { function foo(array $x) {} }

abstract class ParentD { abstract function foo(varray_or_darray $x); }
class ChildD1 extends ParentD { function foo(varray $x) {} }
class ChildD2 extends ParentD { function foo(darray $x) {} }
class ChildD3 extends ParentD { function foo(varray_or_darray $x) {} }
class ChildD4 extends ParentD { function foo(array $x) {} }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
