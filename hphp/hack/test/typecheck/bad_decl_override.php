////file1.php
<?hh

enum E : int {
  A = 3;
}
interface I {
  abstract const vec<E> VALS;
}
////file2.php
<?hh

class C implements I {
  const vec<int> VALS = vec[3];
}
