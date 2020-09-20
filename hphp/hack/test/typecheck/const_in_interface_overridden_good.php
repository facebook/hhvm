<?hh

class C1_1 {
  const int SOME_CONST = 1;
}

class C1_2 extends C1_1 {
  const int SOME_CONST = 2;
}

// ----------

interface I2_1 {
  const int SOME_CONST = 1;
}

class C2_1 implements I2_1 {

}

class C2_2 extends C2_1 {
  const int SOME_CONST = 2;
}

// ----------


interface I3_1 {
  abstract const int SOME_CONST;
}

class C3_1 implements I3_1 {
 const int SOME_CONST = 1;
}
