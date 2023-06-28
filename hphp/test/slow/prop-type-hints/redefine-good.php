<?hh

class Cls1 {}
class Cls2 {}

enum Enum1 : string {
  VAL1 = 'val1';
  VAL2 = 'val2';
  VAL3 = 'val3';
}

enum Enum2 : int {
  VAL1 = 1;
  VAL2 = 2;
  VAL3 = 3;
}

enum Enum3 : string {
  VAL4 = 'val4';
  VAL5 = 'val5';
  VAL6 = 'val6';
}

enum Enum4 : arraykey {
  VAL1 = 1;
  VAL2 = 'val2';
  VAL3 = 3;
}

type Alias1 = int;
type Alias2 = string;
type Alias3 = this;
type Alias4 = Cls1;
type Alias5 = Enum1;
type Alias6 = int;
type Alias7 = Enum3;
type Alias8 = mixed;
type Alias9 = Enum4;
type Alias10 = ?mixed;
type Alias11 = ?Enum4;
type Alias12 = ?int;
type Alias13 = ?Enum2;
type Alias14 = ?Cls1;

class A {
  public int $p1;
  public string $p2;
  public bool $p3;
  public float $p4;
  public AnyArray $p5;
  public resource $p6;
  public nonnull $p7;
  public num $p8;
  public arraykey $p9;
  public vec $p10;
  public dict $p11;
  public keyset $p12;
  public vec_or_dict $p13;
  public AnyArray $p14;
  public varray $p15;
  public darray $p16;
  public varray_or_darray $p17;
  public this $p18;
  public Cls1 $p19;
  public Cls2 $p20;
  public Enum1 $p21;
  public Enum2 $p22;
  public Alias1 $p23;
  public Alias2 $p24;
  public Alias3 $p25;
  public Alias4 $p26;
  public Alias5 $p27;
  public Traversable $p28;
  public $p29;
  public mixed $p30;
  public Enum3 $p31;
  public Alias6 $p32;
  public Alias1 $p33;
  public Alias7 $p34;
  public Enum4 $p35;
  public Alias9 $p36;
  public Enum4 $p37;
  public varray_or_darray $p38;
  public AnyArray $p39;
  public AnyArray $p40;
  public mixed $p41;
  public ?mixed $p42;
  public Alias10 $p43;
  public ?Alias10 $p44;
  public Alias8 $p45;
  public ?Alias8 $p46;
  public Alias11 $p47;
  public ?Alias11 $p48;
  public ?int $p49;
  public ?int $p50;
  public Alias12 $p51;
  public ?Alias12 $p52;
  public ?Alias1 $p53;
  public Alias12 $p54;
  public ?Alias12 $p55;
  public ?int $p56;
  public ?Alias6 $p57;
  public ?Alias6 $p58;
  public ?Alias6 $p59;
  public ?Alias6 $p60;
  public Alias13 $p61;
  public ?Alias13 $p62;
  public ?Enum2 $p63;
  public ?Enum2 $p64;
  public ?int $p65;
  public Alias13 $p66;
  public ?Alias13 $p67;
  public ?int $p68;
  public ?Cls1 $p69;
  public ?Cls1 $p70;
  public Alias14 $p71;
  public ?Alias14 $p72;
  public ?Alias14 $p73;
  public Alias14 $p74;
  public ?Alias14 $p75;
  public ?Alias4 $p76;
  public ?Alias4 $p77;
  <<__Soft>> public int $p78;
  <<__Soft>> public ?int $p79;
  public int $p80;
  public CondEnum1 $p81;
  public CondAlias $p82;
  public int $p83;
  public CondEnum1 $p84;
  public CondAlias $p85;

  public static int $s1;
  public static string $s2;
  public static bool $s3;
  public static float $s4;
  public static AnyArray $s5;
  public static resource $s6;
  public static nonnull $s7;
  public static num $s8;
  public static arraykey $s9;
  public static vec $s10;
  public static dict $s11;
  public static keyset $s12;
  public static vec_or_dict $s13;
  public static AnyArray $s14;
  public static varray $s15;
  public static darray $s16;
  public static varray_or_darray $s17;
  public static this $s18;
  public static Cls1 $s19;
  public static Cls2 $s20;
  public static Enum1 $s21;
  public static Enum2 $s22;
  public static Alias1 $s23;
  public static Alias2 $s24;
  public static Alias3 $s25;
  public static Alias4 $s26;
  public static Alias5 $s27;
  public static Traversable $s28;
  public static $s29;
  public static mixed $s30;
  public static ?int $s31;
  public static int $s32;
  <<__Soft>> public static int $s33;
  public static int $s34;
  <<__Soft>> public static ?int $s35;
  public static int $s36;

  private int $pp1;
  private string $pp2;
  private bool $pp3;
  private float $pp4;
  private AnyArray $pp5;
  private resource $pp6;
  private nonnull $pp7;
  private num $pp8;
  private arraykey $pp9;
  private vec $pp10;
  private dict $pp11;
  private keyset $pp12;
  private vec_or_dict $pp13;
  private AnyArray $pp14;
  private varray $pp15;
  private darray $pp16;
  private varray_or_darray $pp17;
  private this $pp18;
  private Cls1 $pp19;
  private Cls2 $pp20;
  private Enum1 $pp21;
  private Enum2 $pp22;
  private Alias1 $pp23;
  private Alias2 $pp24;
  private Alias3 $pp25;
  private Alias4 $pp26;
  private Alias5 $pp27;
  private Traversable $pp28;
  private $pp29;
  private mixed $pp30;
  private ?int $pp31;
  private int $pp32;
  <<__Soft>> private int $pp33;
  private int $pp34;
  <<__Soft>> private ?int $pp35;
  private int $pp36;
}

class B extends A {
}

class C extends B {
  public int $p1;
  public string $p2;
  public bool $p3;
  public float $p4;
  public AnyArray $p5;
  public resource $p6;
  public nonnull $p7;
  public num $p8;
  public arraykey $p9;
  public vec $p10;
  public dict $p11;
  public keyset $p12;
  public vec_or_dict $p13;
  public AnyArray $p14;
  public varray $p15;
  public darray $p16;
  public varray_or_darray $p17;
  public this $p18;
  public Cls1 $p19;
  public Cls2 $p20;
  public Enum1 $p21;
  public Enum2 $p22;
  public Alias1 $p23;
  public Alias2 $p24;
  public Alias3 $p25;
  public Alias4 $p26;
  public Alias5 $p27;
  public Traversable $p28;
  public $p29;
  public mixed $p30;
  public Enum3 $p31;
  public Alias6 $p32;
  public Alias1 $p33;
  public Alias7 $p34;
  public Enum4 $p35;
  public Alias9 $p36;
  public Enum4 $p37;
  public varray_or_darray $p38;
  public AnyArray $p39;
  public AnyArray $p40;
  public ?int $p49;
  public ?Alias12 $p52;
  <<__Soft>> public int $p78;
  <<__Soft>> public ?int $p79;
  public int $p80;
  public CondEnum1 $p81;
  public CondAlias $p82;
  public int $p83;
  public CondEnum1 $p84;
  public CondAlias $p85;
}

class D extends C {}

class E extends D {
  public Enum2 $p1;
  public Enum1 $p2;
  public AnyArray $p5;
  public varray $p15;
  public darray $p16;
  public varray_or_darray $p17;
  public Alias3 $p18;
  public Alias4 $p19;
  public Alias5 $p21;
  public int $p22;
  public int $p23;
  public string $p24;
  public this $p25;
  public Cls1 $p26;
  public string $p27;
  public mixed $p29;
  public $p30;
  public Enum1 $p31;
  public Alias1 $p32;
  public Alias6 $p33;
  public Alias5 $p34;
  public arraykey $p35;
  public Enum4 $p36;
  public Alias9 $p37;
  public varray_or_darray $p38;
  public AnyArray $p39;
  public AnyArray $p40;
  public ?mixed $p41;
  public mixed $p42;
  public ?Alias10 $p43;
  public Alias10 $p44;
  public ?Alias8 $p45;
  public Alias8 $p46;
  public ?Alias11 $p47;
  public Alias11 $p48;
  public Alias12 $p49;
  public ?Alias12 $p50;
  public ?int $p51;
  public ?int $p52;
  public ?int $p53;
  public ?Alias1 $p54;
  public ?Alias1 $p55;
  public ?Alias6 $p56;
  public ?int $p57;
  public ?Alias1 $p58;
  public Alias12 $p59;
  public ?Alias12 $p60;
  public ?Enum2 $p61;
  public ?Enum2 $p62;
  public Alias13 $p63;
  public ?Alias13 $p64;
  public Alias13 $p65;
  public ?int $p66;
  public ?int $p67;
  public ?Alias13 $p68;
  public Alias14 $p69;
  public ?Alias14 $p70;
  public ?Cls1 $p71;
  public ?Cls1 $p72;
  public Alias14 $p73;
  public ?Alias4 $p74;
  public ?Alias4 $p75;
  public ?Alias14 $p76;
  public Alias14 $p77;
  public CondEnum1 $p80;
  public int $p81;
  public int $p82;
  public CondAlias $p83;
  public CondAlias $p84;
  public CondEnum1 $p85;

  public static string $s1;
  public static int $s2;
  public static float $s3;
  public static bool $s4;
  public static vec $s5;
  public static string $s6;
  public static dict $s7;
  public static bool $s8;
  public static vec $s9;
  public static keyset $s10;
  public static AnyArray $s11;
  public static vec $s12;
  public static AnyArray $s13;
  public static int $s14;
  public static vec $s15;
  public static dict $s16;
  public static vec_or_dict $s17;
  public static Enum1 $s18;
  public static Enum2 $s19;
  public static Cls1 $s20;
  public static Enum2 $s21;
  public static Enum1 $s22;
  public static Alias5 $s23;
  public static Alias4 $s24;
  public static Alias3 $s25;
  public static Alias2 $s26;
  public static Alias1 $s27;
  public static KeyedTraversable $s28;
  public static int $s29;
  public static string $s30;
  public static int $s31;
  public static ?int $s32;
  public static int $s33;
  <<__Soft>> public static int $s34;
  public static int $s35;
  <<__Soft>> public static ?int $s36;

  private string $pp1;
  private int $pp2;
  private float $pp3;
  private bool $pp4;
  private vec $pp5;
  private string $pp6;
  private dict $pp7;
  private bool $pp8;
  private vec $pp9;
  private keyset $pp10;
  private AnyArray $pp11;
  private vec $pp12;
  private AnyArray $pp13;
  private int $pp14;
  private vec $pp15;
  private dict $pp16;
  private vec_or_dict $pp17;
  private Enum1 $pp18;
  private Enum2 $pp19;
  private Cls1 $pp20;
  private Enum2 $pp21;
  private Enum1 $pp22;
  private Alias5 $pp23;
  private Alias4 $pp24;
  private Alias3 $pp25;
  private Alias2 $pp26;
  private Alias1 $pp27;
  private KeyedTraversable $pp28;
  private int $pp29;
  private string $pp30;
  private int $pp31;
  private ?int $pp32;
  private int $pp33;
  <<__Soft>> private int $pp34;
  private int $pp35;
  <<__Soft>> private ?int $pp36;
}

class F extends E {
}

class G extends F {
  public Alias1 $p1;
  public Enum3 $p2;
  public AnyArray $p5;
  public varray $p15;
  public darray $p16;
  public Alias6 $p23;
  public $p29;
  public AnyArray $p39;
  public AnyArray $p40;
  public mixed $p43;
  public ?mixed $p44;
  public mixed $p45;
  public ?mixed $p46;
  public ?arraykey $p47;
  public ?arraykey $p48;
  public ?Alias12 $p49;
  public Alias12 $p50;
  public ?Alias1 $p51;

  public static function test() :mixed{
    new A();
    new B();
    new C();
    new D();
    new E();
    new F();
    new G();
  }
}
<<__EntryPoint>>
function main_entry(): void {

  if (__hhvm_intrinsics\launder_value(true)) {
    include 'redefine1.inc';
  } else {
    include 'redefine2.inc';
  }

  G::test();
  G::test();
  echo "DONE\n";
}
