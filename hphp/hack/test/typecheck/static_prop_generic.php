//// filea.php
<?hh

abstract class X {}

trait T1<T as X> {
  <<__LateInit>> private static T $p2;
}
//// fileb.php
<?hh

trait T2<T as X> {
  <<__LateInit>> private static T $p3;
}
abstract class A<T as X> {
  <<__LateInit>> private static T $p1;
  use T1<T>;
  use T2<T>;
}
