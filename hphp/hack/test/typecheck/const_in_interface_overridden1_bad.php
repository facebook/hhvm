<?hh

interface I {
  const int SOME_CONST = 1;
}
abstract class C implements I {
 const int SOME_CONST = 2;
}
