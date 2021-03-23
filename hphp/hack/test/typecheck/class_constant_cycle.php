<?hh

/* cycles without inheritance */
class C0 {
  const int A = C0::B;
  const int B = C0::A;
}

class D0 {
  const int A = self::C;
  const int B = self::A;
  const int C = self::B;
}

class C {
  const int A = D::X + Z::L;
  const int B = self::A;

  const int W = self::WW;
  const int WW = Z::L + Z::Y;

  const int C_SELF = self::C_SELF;
  const int C_SELF2 = C::C_SELF2;
}

class Z {
  const int L = 42;
  const int Y = 42;
}

class D {
  const int X = C::B;
}

/* cycles with inheritance */
class CE {
  const int X = CF::X;
}

class CF extends CE {}

/* Same constant name but no cycle */
class C_OK { const int A = 42; }
class D_OK { const int A = C_OK::A; } // No error here

/* Indirect cycle: we report only in E_KO, not in F_OK */
class E_KO {
    const int A = E_KO::B; // Error here
    const int B = E_KO::A; // Error here
}

class F_OK {
    const int A = E_KO::A; // No error here.
}
