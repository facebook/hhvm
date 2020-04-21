//// A.php
<?hh
abstract class A<T> { public T $x; }

//// B.php
<?hh
class B extends A<?int> {}

//// C.php
<?hh
class C extends A<int> {}
