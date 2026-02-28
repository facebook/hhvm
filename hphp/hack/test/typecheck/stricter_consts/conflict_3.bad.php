<?hh

trait T1 {
  const int X = 4;
}
trait T2 {
  const int X = 5;
}
trait T3 {
  const int X = 6;
}
class C {
  use T1, T2, T3;
}
