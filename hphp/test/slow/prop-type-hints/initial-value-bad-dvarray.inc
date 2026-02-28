<?hh

class A {
  public darray $p2 = vec[1, 2, 3];
  public AnyArray $p3 = vec[1, 2, 3];
  public darray $p4 = vec[1, 2, 3];
  public AnyArray $p5 = dict['a' => 1, 'b' => 2];
  public varray $p6 = dict['a' => 1, 'b' => 2];

  public darray $p8 = CONDCONST6;
  public AnyArray $p9= CONDCONST4;
  public darray $p10 = CONDCONST4;
  public AnyArray $p11 = CONDCONST5;
  public varray $p12 = CONDCONST5;

  public DAlias $p14 = CONDCONST6;
  public AAlias $p15 = CONDCONST4;
  public DAlias $p16 = CONDCONST4;
  public AAlias $p17 = CONDCONST5;
  public VAlias $p18 = CONDCONST5;

  public static darray $s2 = vec[1, 2, 3];
  public static AnyArray $s3 = vec[1, 2, 3];
  public static darray $s4 = vec[1, 2, 3];
  public static AnyArray $s5 = dict['a' => 1, 'b' => 2];
  public static varray $s6 = dict['a' => 1, 'b' => 2];

  public static darray $s8 = CONDCONST6;
  public static AnyArray $s9 = CONDCONST4;
  public static darray $s10 = CONDCONST4;
  public static AnyArray $s11 = CONDCONST5;
  public static varray $s12 = CONDCONST5;

  public static DAlias $s14 = CONDCONST6;
  public static AAlias $s15 = CONDCONST4;
  public static DAlias $s16 = CONDCONST4;
  public static AAlias $s17 = CONDCONST5;
  public static VAlias $s18 = CONDCONST5;
}

class B extends A {
  public static function test() {
    new B();
  }
}
