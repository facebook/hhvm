<?hh

type Alias1 = int;
type Alias2 = string;
type Alias3 = Enum1;
type Alias4 = Enum2;
type Alias5 = Enum3;
type Alias6 = Vector;
type Alias7 = CondEnum1;
type Alias8 = ?int;
type Alias9 = ?Enum1;
type Alias10 = ?Vector;
type Alias11 = ?CondEnum1;
type Alias12 = ?Traversable;

const INTCONST = 123;
const STRCONST = 'abc';
const BOOLCONST = false;

class A {
  public int $p1 = 'abc';
  public string $p2 = 123;
  public bool $p3 = 123;
  public float $p4 = false;
  public AnyArray $p5 = vec[1, 2, 3];
  public resource $p6 = null;
  public nonnull $p7 = null;
  public num $p8 = true;
  public arraykey $p9 = keyset[];
  public vec $p10 = dict[1 => 100, 2 => 200, 3 => 300];
  public dict $p11 = vec['a', 'b'];
  public keyset $p12 = vec['a', 'b', 'c'];
  public vec_or_dict $p13 = vec['a', 'b',' c'];
  public AnyArray $p14 = null;
  public varray $p15 = vec['val1', 'val2'];
  public darray $p16 = dict['a' => 100, 'b' => 200];
  public varray_or_darray $p17 = vec[9, 8, 7];
  public Vector $p18 = Map{};
  public Map $p19 = Vector{};
  public Enum1 $p20 = 123;
  public Enum2 $p21 = 'abc';
  public Enum3 $p22 = false;
  public Alias1 $p23 = 'def';
  public Alias2 $p24 = 123;
  public Alias3 $p25 = 123;
  public Alias4 $p26 = 'ghi';
  public Alias6 $p27 = Map{};
  public Alias7 $p28 = false;
  public Alias8 $p29 = true;
  public Alias9 $p30 = 123;
  public Alias10 $p31 = Map{};
  public Alias11 $p32 = 'abc';
  public CondAlias $p33 = 'def';
  public CondEnum1 $p34 = 'zyz';
  public Traversable $p35 = 123;
  public Traversable $p36 = false;
  public Traversable $p37 = 3.14;
  public Alias12 $p38 = 123;
  public Alias12 $p39 = false;
  public Alias12 $p40 = 3.14;

  public int $p41 = STRCONST;
  public string $p42 = INTCONST;
  public Vector $p43 = Map{STRCONST => INTCONST};
  public Map $p44 = Vector{STRCONST};
  public arraykey $p45 = BOOLCONST;
  public vec $p46 = keyset[INTCONST, STRCONST, INTCONST];
  public dict $p47 = dict[STRCONST => INTCONST, INTCONST => STRCONST];
  public AnyArray $p48 = vec[INTCONST, STRCONST, INTCONST];
  public Enum1 $p49 = INTCONST;
  public Enum2 $p50 = STRCONST;
  public Enum3 $p51 = vec[INTCONST];
  public Alias1 $p52 = STRCONST;
  public Alias2 $p53 = INTCONST;
  public Alias3 $p54 = INTCONST;
  public Alias4 $p55 = STRCONST;
  public Alias5 $p56 = vec[STRCONST];
  public Alias6 $p57 = Pair{INTCONST, STRCONST};
  public Alias7 $p58 = STRCONST;
  public Alias8 $p59 = STRCONST;
  public Alias9 $p60 = INTCONST;
  public Alias10 $p61 = Pair{INTCONST, STRCONST};
  public Alias11 $p62 = STRCONST;
  public CondAlias $p63 = STRCONST;
  public CondEnum1 $p64 = STRCONST;

  public int $p65 = CONDCONST2;
  public string $p66 = CONDCONST1;
  public Vector $p67 = Pair{CONDCONST1, CONDCONST2};
  public Map $p68 = Vector{CONDCONST2, CONDCONST1};
  public arraykey $p69 = CONDCONST3;
  public vec $p70 = keyset[CONDCONST1, CONDCONST2, CONDCONST1];
  public dict $p71 = vec[CONDCONST2, CONDCONST1];
  public AnyArray $p72 = vec[CONDCONST1, CONDCONST2, CONDCONST1];
  public Enum1 $p73 = CONDCONST1;
  public Enum2 $p74 = CONDCONST2;
  public Enum3 $p75 = CONDCONST3;
  public Alias1 $p76 = CONDCONST2;
  public Alias2 $p77 = CONDCONST1;
  public Alias3 $p78 = CONDCONST1;
  public Alias4 $p79 = CONDCONST2;
  public Alias6 $p80 = Pair{CONDCONST1, CONDCONST2};
  public Alias7 $p81 = CONDCONST2;
  public Alias8 $p82 = CONDCONST2;
  public Alias9 $p83 = CONDCONST1;
  public Alias10 $p84 = Pair{CONDCONST1, CONDCONST2};
  public Alias11 $p85 = CONDCONST2;
  public CondAlias $p86 = CONDCONST2;
  public CondEnum1 $p87 = CONDCONST2;

  public static int $s1 = 'abc';
  public static string $s2 = 123;
  public static bool $s3 = 123;
  public static float $s4 = false;
  public static AnyArray $s5 = vec[1, 2, 3];
  public static resource $s6 = null;
  public static nonnull $s7 = null;
  public static num $s8 = true;
  public static arraykey $s9 = keyset[];
  public static vec $s10 = dict[1 => 100, 2 => 200, 3 => 300];
  public static dict $s11 = vec['a', 'b'];
  public static keyset $s12 = vec['a', 'b', 'c'];
  public static vec_or_dict $s13 = vec['a', 'b',' c'];
  public static AnyArray $s14 = null;
  public static varray $s15 = vec['val1', 'val2'];
  public static darray $s16 = dict['a' => 100, 'b' => 200];
  public static varray_or_darray $s17 = vec[9, 8, 7];
  public static Vector $s18 = Map{};
  public static Map $s19 = Vector{};
  public static Enum1 $s20 = 123;
  public static Enum2 $s21 = 'abc';
  public static Enum3 $s22 = false;
  public static Alias1 $s23 = 'def';
  public static Alias2 $s24 = 123;
  public static Alias3 $s25 = 123;
  public static Alias4 $s26 = 'ghi';
  public static Alias6 $s27 = Map{};
  public static Alias7 $s28 = false;
  public static Alias8 $s29 = true;
  public static Alias9 $s30 = 123;
  public static Alias10 $s31 = Map{};
  public static Alias11 $s32 = 'abc';
  public static CondAlias $s33 = 'def';
  public static CondEnum1 $s34 = 'zyz';
  public static Traversable $s35 = 123;
  public static Traversable $s36 = false;
  public static Traversable $s37 = 3.14;
  public static Alias12 $s38 = 123;
  public static Alias12 $s39 = false;
  public static Alias12 $s40 = 3.14;

  public static int $s41 = STRCONST;
  public static string $s42 = INTCONST;
  public static Vector $s43 = Map{STRCONST => INTCONST};
  public static Map $s44 = Vector{STRCONST};
  public static arraykey $s45 = STRCONST;
  public static vec $s46 = keyset[INTCONST, STRCONST, INTCONST];
  public static dict $s47 = dict[STRCONST => INTCONST, INTCONST => STRCONST];
  public static AnyArray $s48 = vec[INTCONST, STRCONST, INTCONST];
  public static Enum1 $s49 = INTCONST;
  public static Enum2 $s50 = STRCONST;
  public static Enum3 $s51 = vec[INTCONST];
  public static Alias1 $s52 = STRCONST;
  public static Alias2 $s53 = INTCONST;
  public static Alias3 $s54 = INTCONST;
  public static Alias4 $s55 = STRCONST;
  public static Alias5 $s56 = vec[STRCONST];
  public static Alias6 $s57 = Pair{INTCONST, STRCONST};
  public static Alias7 $s58 = STRCONST;
  public static Alias8 $s59 = STRCONST;
  public static Alias9 $s60 = INTCONST;
  public static Alias10 $s61 = Pair{INTCONST, STRCONST};
  public static Alias11 $s62 = STRCONST;
  public static CondAlias $s63 = STRCONST;
  public static CondEnum1 $s64 = STRCONST;

  public static int $s65 = CONDCONST2;
  public static string $s66 = CONDCONST1;
  public static Vector $s67 = Pair{CONDCONST1, CONDCONST2};
  public static Map $s68 = Vector{CONDCONST2, CONDCONST1};
  public static arraykey $s69 = CONDCONST3;
  public static vec $s70 = keyset[CONDCONST1, CONDCONST2, CONDCONST1];
  public static dict $s71 = vec[CONDCONST2, CONDCONST1];
  public static AnyArray $s72 = vec[CONDCONST1, CONDCONST2, CONDCONST1];
  public static Enum1 $s73 = CONDCONST1;
  public static Enum2 $s74 = CONDCONST2;
  public static Enum3 $s75 = CONDCONST3;
  public static Alias1 $s76 = CONDCONST2;
  public static Alias2 $s77 = CONDCONST1;
  public static Alias3 $s78 = CONDCONST1;
  public static Alias4 $s79 = CONDCONST2;
  public static Alias6 $s80 = Pair{CONDCONST1, CONDCONST2};
  public static Alias7 $s81 = CONDCONST2;
  public static Alias8 $s82 = CONDCONST2;
  public static Alias9 $s83 = CONDCONST1;
  public static Alias10 $s84 = Pair{CONDCONST1, CONDCONST2};
  public static Alias11 $s85 = CONDCONST2;
  public static CondAlias $s86 = CONDCONST2;
  public static CondEnum1 $s87 = CONDCONST2;

  public static function test() {
    new A();
  }
}

class B {
  public int $p1 = 'abc';
  public Vector $p2 = Map{};
  public int $p3 = STRCONST;
  public int $p4 = CONDCONST2;

  public static int $s1 = 'abc';
  public static string $s2 = 123;

  private int $pp1 = 'abc';
  private string $pp2 = 123;
}

class C extends B {
  public int $p1 = 123;
  public Vector $p2 = Vector{};
  public int $p3 = INTCONST;
  public int $p4 = CONDCONST1;

  public static int $s1 = 'abc';
  public static string $s2 = 'abc';

  private int $pp1 = 'abc';
  private string $pp2 = 'abc';

  public static function test() {
    new C();
  }
}
