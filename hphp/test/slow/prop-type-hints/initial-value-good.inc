<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

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

class A {
  public int $p1 = 123;
  public string $p2 = 'abc';
  public bool $p3 = false;
  public float $p4 = 3.14;
  public varray $p5a = vec[1, 2, 3]; /* TODO: remove */
  public varray $p5b = vec[1, 2, 3];
  public darray $p5c = dict[1 => 'a', 2 => 'b', 3 => 'c'];
  public ?resource $p6 = null;
  public nonnull $p7 = 123;
  public num $p8 = 456;
  public arraykey $p9 = 'abc';
  public arraykey $p10 = 123;
  public vec $p11 = vec[1, 2, 3];
  public dict $p12 = dict['a' => 100, 'b' => 200];
  public keyset $p13 = keyset['a', 'b', 'c'];
  public vec_or_dict $p14 = vec['a', 'b',' c'];
  public vec_or_dict $p15 = dict['a' => 100, 'b' => 200];
  public AnyArray $p16 = vec[100, 200];
  public AnyArray $p17 = vec[100, 200];
  public AnyArray $p18 = dict[1 => 100, 2 => 200];
  public AnyArray $p19 = keyset[100, 200];
  public varray $p20 = vec['val1', 'val2'];
  public darray $p21 = dict['a' => 100, 'b' => 200];
  public darray $p22 = dict['a' => 100, 'b' => 200];
  public varray $p23 = vec['val1', 'val2'];
  public varray_or_darray $p24 = vec[9, 8, 7];
  public varray_or_darray $p25 = dict['a' => 100, 'b' => 200];
  public varray_or_darray $p26 = dict['a' => 100, 'b' => 200]; /* TODO: remove */
  public Vector $p27 = Vector{};
  public Map $p28 = Map{};
  public Enum1 $p29 = 'abc';
  public Enum2 $p30 = 123;
  public Enum3 $p31 = 123;
  public Enum3 $p32 = 'abc';
  public Alias1 $p33 = 456;
  public Alias2 $p34 = 'def';
  public Alias3 $p35 = 'xyz';
  public Alias4 $p36 = 789;
  public Alias5 $p37 = 'xyz';
  public Alias5 $p38 = 789;
  public Alias6 $p39 = Vector{};
  public Alias7 $p40 = 700;
  public Alias8 $p41 = 800;
  public Alias8 $p42 = null;
  public Alias9 $p43 = 'azaz';
  public Alias9 $p44 = null;
  public Alias10 $p45 = Vector{};
  public Alias10 $p46 = null;
  public Alias11 $p47 = 500;
  public Alias11 $p48 = null;
  public CondAlias $p49 = 818;
  public CondEnum1 $p50 = 181;
  public Traversable $p51 = vec[];
  public Traversable $p52 = dict[];
  public Traversable $p53 = vec[];
  public Alias12 $p54 = vec[];
  public Alias12 $p55 = dict[];
  public Alias12 $p56 = vec[];
  public Alias12 $p57 = null;

  public int $p58 = INTCONST;
  public string $p59 = STRCONST;
  public Vector $p60 = Vector{INTCONST, STRCONST};
  public Map $p61 = Map{STRCONST => INTCONST};
  public arraykey $p62 = STRCONST;
  public vec $p63 = vec[INTCONST, STRCONST, INTCONST];
  public dict $p64 = dict[STRCONST => INTCONST, INTCONST => STRCONST];
  public varray $p65 = vec[INTCONST, STRCONST, INTCONST];
  public Enum1 $p66 = STRCONST;
  public Enum2 $p67 = INTCONST;
  public Enum3 $p68 = INTCONST;
  public Enum3 $p69 = STRCONST;
  public Alias1 $p70 = INTCONST;
  public Alias2 $p71 = STRCONST;
  public Alias3 $p72 = STRCONST;
  public Alias4 $p73 = INTCONST;
  public Alias5 $p74 = STRCONST;
  public Alias5 $p75 = INTCONST;
  public Alias6 $p76 = Vector{INTCONST, STRCONST};
  public Alias7 $p77 = INTCONST;
  public Alias8 $p78 = INTCONST;
  public Alias9 $p79 = STRCONST;
  public Alias10 $p80 = Vector{INTCONST, STRCONST};
  public Alias11 $p81 = INTCONST;
  public CondAlias $p82 = INTCONST;
  public CondEnum1 $p83 = INTCONST;

  public int $p84 = CONDCONST1;
  public string $p85 = CONDCONST2;
  public Vector $p86 = Vector{CONDCONST1, CONDCONST2};
  public Map $p87 = Map{CONDCONST2 => CONDCONST1};
  public arraykey $p88 = CONDCONST2;
  public vec $p89 = vec[CONDCONST1, CONDCONST2, CONDCONST1];
  public dict $p90 = dict[CONDCONST2 => CONDCONST1, CONDCONST1 => CONDCONST2];
  public varray $p91 = vec[CONDCONST1, CONDCONST2, CONDCONST1];
  public Enum1 $p92 = CONDCONST2;
  public Enum2 $p93 = CONDCONST1;
  public Enum3 $p94 = CONDCONST1;
  public Enum3 $p95 = CONDCONST2;
  public Alias1 $p96 = CONDCONST1;
  public Alias2 $p97 = CONDCONST2;
  public Alias3 $p98 = CONDCONST2;
  public Alias4 $p99 = CONDCONST1;
  public Alias5 $p100 = CONDCONST2;
  public Alias5 $p101 = CONDCONST1;
  public Alias6 $p102 = Vector{CONDCONST1, CONDCONST2};
  public Alias7 $p103 = CONDCONST1;
  public Alias8 $p104 = CONDCONST1;
  public Alias9 $p105 = CONDCONST2;
  public Alias10 $p106 = Vector{CONDCONST1, CONDCONST2};
  public Alias11 $p107 = CONDCONST1;
  public CondAlias $p108 = CONDCONST1;
  public CondEnum1 $p109 = CONDCONST1;

  public ?int $opt1 = null;
  public ?string $opt2 = null;
  public ?bool $opt3 = null;
  public ?float $opt4 = null;
  public ?AnyArray $opt5 = null;
  public ?resource $opt6 = null;
  public ?nonnull $opt7 = null;
  public ?num $opt8 = null;
  public ?arraykey $opt9 = null;
  public ?vec $opt11 = null;
  public ?dict $opt12 = null;
  public ?keyset $opt13 = null;
  public ?vec_or_dict $opt14 = null;
  public ?AnyArray $opt16 = null;
  public ?varray $opt20 = null;
  public ?darray $opt22 = null;
  public ?varray_or_darray $opt24 = null;
  public ?Vector $opt27 = null;
  public ?Map $opt28 = null;
  public ?Enum1 $opt29 = null;
  public ?Enum2 $opt30 = null;
  public ?Enum3 $opt31 = null;
  public ?Alias1 $opt33 = null;
  public ?Alias2 $opt34 = null;
  public ?Alias3 $opt35 = null;
  public ?Alias4 $opt36 = null;
  public ?Alias5 $opt38 = null;
  public ?Alias6 $opt39 = null;
  public ?Alias7 $opt40 = null;
  public ?Alias8 $opt42 = null;
  public ?Alias9 $opt43 = null;
  public ?Alias10 $opt45 = null;
  public ?Alias11 $opt47 = null;
  public ?CondAlias $opt49 = null;
  public ?CondEnum1 $opt50 = null;
  public ?Traversable $opt51 = null;
  public ?Alias12 $opt54 = null;

  public static int $s1 = 123;
  public static string $s2 = 'abc';
  public static bool $s3 = false;
  public static float $s4 = 3.14;
  public static varray $s5a = vec[1, 2, 3];
  public static varray $s5b = vec[1, 2, 3];
  public static darray $s5c = dict[1 => 'a', 2 => 'b', 3 => 'c'];
  public static ?resource $s6 = null;
  public static nonnull $s7 = 123;
  public static num $s8 = 456;
  public static arraykey $s9 = 'abc';
  public static arraykey $s10 = 123;
  public static vec $s11 = vec[1, 2, 3];
  public static dict $s12 = dict['a' => 100, 'b' => 200];
  public static keyset $s13 = keyset['a', 'b', 'c'];
  public static vec_or_dict $s14 = vec['a', 'b',' c'];
  public static vec_or_dict $s15 = dict['a' => 100, 'b' => 200];
  public static AnyArray $s16 = vec[100, 200];
  public static AnyArray $s17 = vec[100, 200];
  public static AnyArray $s18 = dict[1 => 100, 2 => 200];
  public static AnyArray $s19 = keyset[100, 200];
  public static varray $s20 = vec['val1', 'val2'];
  public static darray $s21 = dict['a' => 100, 'b' => 200];
  public static darray $s22 = dict['a' => 100, 'b' => 200];
  public static varray $s23 = vec['val1', 'val2'];
  public static varray_or_darray $s24 = vec[9, 8, 7];
  public static varray_or_darray $s25 = dict['a' => 100, 'b' => 200];
  public static varray_or_darray $s26 = dict['a' => 100, 'b' => 200]; /* TODO: delete */
  public static Vector $s27 = Vector{};
  public static Map $s28 = Map{};
  public static Enum1 $s29 = 'abc';
  public static Enum2 $s30 = 123;
  public static Enum3 $s31 = 123;
  public static Enum3 $s32 = 'abc';
  public static Alias1 $s33 = 456;
  public static Alias2 $s34 = 'def';
  public static Alias3 $s35 = 'xyz';
  public static Alias4 $s36 = 789;
  public static Alias5 $s37 = 'xyz';
  public static Alias5 $s38 = 789;
  public static Alias6 $s39 = Vector{};
  public static Alias7 $s40 = 700;
  public static Alias8 $s41 = 800;
  public static Alias8 $s42 = null;
  public static Alias9 $s43 = 'azaz';
  public static Alias9 $s44 = null;
  public static Alias10 $s45 = Vector{};
  public static Alias10 $s46 = null;
  public static Alias11 $s47 = 500;
  public static Alias11 $s48 = null;
  public static CondAlias $s49 = 818;
  public static CondEnum1 $s50 = 181;
  public static Traversable $s51 = vec[];
  public static Traversable $s52 = dict[];
  public static Traversable $s53 = vec[];
  public static Alias12 $s54 = vec[];
  public static Alias12 $s55 = dict[];
  public static Alias12 $s56 = vec[];
  public static Alias12 $s57 = null;

  public static int $s58 = INTCONST;
  public static string $s59 = STRCONST;
  public static Vector $s60 = Vector{INTCONST, STRCONST};
  public static Map $s61 = Map{STRCONST => INTCONST};
  public static arraykey $s62 = STRCONST;
  public static vec $s63 = vec[INTCONST, STRCONST, INTCONST];
  public static dict $s64 = dict[STRCONST => INTCONST, INTCONST => STRCONST];
  public static varray $s65 = vec[INTCONST, STRCONST, INTCONST];
  public static Enum1 $s66 = STRCONST;
  public static Enum2 $s67 = INTCONST;
  public static Enum3 $s68 = INTCONST;
  public static Enum3 $s69 = STRCONST;
  public static Alias1 $s70 = INTCONST;
  public static Alias2 $s71 = STRCONST;
  public static Alias3 $s72 = STRCONST;
  public static Alias4 $s73 = INTCONST;
  public static Alias5 $s74 = STRCONST;
  public static Alias5 $s75 = INTCONST;
  public static Alias6 $s76 = Vector{INTCONST, STRCONST};
  public static Alias7 $s77 = INTCONST;
  public static Alias8 $s78 = INTCONST;
  public static Alias9 $s79 = STRCONST;
  public static Alias10 $s80 = Vector{INTCONST, STRCONST};
  public static Alias11 $s81 = INTCONST;
  public static CondAlias $s82 = INTCONST;
  public static CondEnum1 $s83 = INTCONST;

  public static int $s84 = CONDCONST1;
  public static string $s85 = CONDCONST2;
  public static Vector $s86 = Vector{CONDCONST1, CONDCONST2};
  public static Map $s87 = Map{CONDCONST2 => CONDCONST1};
  public static arraykey $s88 = CONDCONST2;
  public static vec $s89 = vec[CONDCONST1, CONDCONST2, CONDCONST1];
  public static dict $s90 = dict[CONDCONST2 => CONDCONST1, CONDCONST1 => CONDCONST2];
  public static varray $s91 = vec[CONDCONST1, CONDCONST2, CONDCONST1];
  public static Enum1 $s92 = CONDCONST2;
  public static Enum2 $s93 = CONDCONST1;
  public static Enum3 $s94 = CONDCONST1;
  public static Enum3 $s95 = CONDCONST2;
  public static Alias1 $s96 = CONDCONST1;
  public static Alias2 $s97 = CONDCONST2;
  public static Alias3 $s98 = CONDCONST2;
  public static Alias4 $s99 = CONDCONST1;
  public static Alias5 $s100 = CONDCONST2;
  public static Alias5 $s101 = CONDCONST1;
  public static Alias6 $s102 = Vector{CONDCONST1, CONDCONST2};
  public static Alias7 $s103 = CONDCONST1;
  public static Alias8 $s104 = CONDCONST1;
  public static Alias9 $s105 = CONDCONST2;
  public static Alias10 $s106 = Vector{CONDCONST1, CONDCONST2};
  public static Alias11 $s107 = CONDCONST1;
  public static CondAlias $s108 = CONDCONST1;
  public static CondEnum1 $s109 = CONDCONST1;

  public static ?int $sopt1 = null;
  public static ?string $sopt2 = null;
  public static ?bool $sopt3 = null;
  public static ?float $sopt4 = null;
  public static ?AnyArray $sopt5 = null;
  public static ?resource $sopt6 = null;
  public static ?nonnull $sopt7 = null;
  public static ?num $sopt8 = null;
  public static ?arraykey $sopt9 = null;
  public static ?vec $sopt11 = null;
  public static ?dict $sopt12 = null;
  public static ?keyset $sopt13 = null;
  public static ?vec_or_dict $sopt14 = null;
  public static ?AnyArray $sopt16 = null;
  public static ?varray $sopt20 = null;
  public static ?darray $sopt22 = null;
  public static ?varray_or_darray $sopt24 = null;
  public static ?Vector $sopt27 = null;
  public static ?Map $sopt28 = null;
  public static ?Enum1 $sopt29 = null;
  public static ?Enum2 $sopt30 = null;
  public static ?Enum3 $sopt31 = null;
  public static ?Alias1 $sopt33 = null;
  public static ?Alias2 $sopt34 = null;
  public static ?Alias3 $sopt35 = null;
  public static ?Alias4 $sopt36 = null;
  public static ?Alias5 $sopt38 = null;
  public static ?Alias6 $sopt39 = null;
  public static ?Alias7 $sopt40 = null;
  public static ?Alias8 $sopt42 = null;
  public static ?Alias9 $sopt43 = null;
  public static ?Alias10 $sopt45 = null;
  public static ?Alias11 $sopt47 = null;
  public static ?CondAlias $sopt49 = null;
  public static ?CondEnum1 $sopt50 = null;
  public static ?Traversable $sopt51 = null;
  public static ?Alias12 $sopt54 = null;

  public static function test() {
    new A();
  }
}
