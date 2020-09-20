<?hh

interface I1 {
  const int SOME_CONST = 1;
}

interface I2 extends I1{

}

class C implements I2 {
 const int SOME_CONST = 2;
}
