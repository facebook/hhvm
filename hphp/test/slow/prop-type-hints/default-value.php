<?hh

class Cls1 {}
enum Enum1 : int {
  VAL1 = 1;
  VAL2 = 2;
  VAL3 = 3;
}

enum Enum2 : string {
  VAL1 = 'VAL1';
  VAL2 = 'VAL2';
  VAL3 = 'VAL3';
}

enum Enum3 : arraykey {
  VAL1 = 1;
  VAL2 = 'VAL2';
  VAL3 = 3;
}

type Alias1 = int;
type Alias2 = string;
type Alias3 = vec;
type Alias4 = Enum2;
type Alias5 = ?dict;
type Alias6 = ?Enum2;

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
  public Traversable $p19;
  public Cls1 $p20;
  public Enum1 $p21;
  public Enum2 $p22;
  public Enum3 $p23;
  public Alias1 $p24;
  public Alias2 $p25;
  public Alias3 $p26;
  public Alias4 $p27;
  public Alias5 $p28;
  public Alias6 $p29;
  public CondEnum1 $p30;
  public CondAlias $p31;

  public ?int $opt1;
  public ?string $opt2;
  public ?bool $opt3;
  public ?float $opt4;
  public ?AnyArray $opt5;
  public ?resource $opt6;
  public ?nonnull $opt7;
  public ?num $opt8;
  public ?arraykey $opt9;
  public ?vec $opt10;
  public ?dict $opt11;
  public ?keyset $opt12;
  public ?vec_or_dict $opt13;
  public ?AnyArray $opt14;
  public ?varray $opt15;
  public ?darray $opt16;
  public ?varray_or_darray $opt17;
  public ?this $opt18;
  public ?Traversable $opt19;
  public ?Cls1 $opt20;
  public ?Enum1 $opt21;
  public ?Enum2 $opt22;
  public ?Enum3 $opt23;
  public ?Alias1 $opt24;
  public ?Alias2 $opt25;
  public ?Alias3 $opt26;
  public ?Alias4 $opt27;
  public ?Alias5 $opt28;
  public ?Alias6 $opt29;
  public ?CondEnum1 $opt30;
  public ?CondAlias $opt31;

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
  public static Traversable $s19;
  public static Cls1 $s20;
  public static Enum1 $s21;
  public static Enum2 $s22;
  public static Enum3 $s23;
  public static Alias1 $s24;
  public static Alias2 $s25;
  public static Alias3 $s26;
  public static Alias4 $s27;
  public static Alias5 $s28;
  public static Alias6 $s29;
  public static CondEnum1 $s30;
  public static CondAlias $s31;

  public static ?int $sopt1;
  public static ?string $sopt2;
  public static ?bool $sopt3;
  public static ?float $sopt4;
  public static ?AnyArray $sopt5;
  public static ?resource $sopt6;
  public static ?nonnull $sopt7;
  public static ?num $sopt8;
  public static ?arraykey $sopt9;
  public static ?vec $sopt10;
  public static ?dict $sopt11;
  public static ?keyset $sopt12;
  public static ?vec_or_dict $sopt13;
  public static ?AnyArray $sopt14;
  public static ?varray $sopt15;
  public static ?darray $sopt16;
  public static ?varray_or_darray $sopt17;
  public static ?this $sopt18;
  public static ?Traversable $sopt19;
  public static ?Cls1 $sopt20;
  public static ?Enum1 $sopt21;
  public static ?Enum2 $sopt22;
  public static ?Enum3 $sopt23;
  public static ?Alias1 $sopt24;
  public static ?Alias2 $sopt25;
  public static ?Alias3 $sopt26;
  public static ?Alias4 $sopt27;
  public static ?Alias5 $sopt28;
  public static ?Alias6 $sopt29;
  public static ?CondEnum1 $sopt30;
  public static ?CondAlias $sopt31;
}

function test($x) :mixed{
  var_dump($x->p1);
  var_dump($x->p2);
  var_dump($x->p3);
  var_dump($x->p4);
  var_dump($x->p5);
  var_dump($x->p6);
  var_dump($x->p7);
  var_dump($x->p8);
  var_dump($x->p9);
  var_dump($x->p10);
  var_dump($x->p11);
  var_dump($x->p12);
  var_dump($x->p13);
  var_dump($x->p14);
  var_dump($x->p15);
  var_dump($x->p16);
  var_dump($x->p17);
  var_dump($x->p18);
  var_dump($x->p19);
  var_dump($x->p20);
  var_dump($x->p21);
  var_dump($x->p22);
  var_dump($x->p23);
  var_dump($x->p24);
  var_dump($x->p25);
  var_dump($x->p26);
  var_dump($x->p27);
  var_dump($x->p28);
  var_dump($x->p29);
  var_dump($x->p30);
  var_dump($x->p31);

  var_dump($x->opt1);
  var_dump($x->opt2);
  var_dump($x->opt3);
  var_dump($x->opt4);
  var_dump($x->opt5);
  var_dump($x->opt6);
  var_dump($x->opt7);
  var_dump($x->opt8);
  var_dump($x->opt9);
  var_dump($x->opt10);
  var_dump($x->opt11);
  var_dump($x->opt12);
  var_dump($x->opt13);
  var_dump($x->opt14);
  var_dump($x->opt15);
  var_dump($x->opt16);
  var_dump($x->opt17);
  var_dump($x->opt18);
  var_dump($x->opt19);
  var_dump($x->opt20);
  var_dump($x->opt21);
  var_dump($x->opt22);
  var_dump($x->opt23);
  var_dump($x->opt24);
  var_dump($x->opt25);
  var_dump($x->opt26);
  var_dump($x->opt27);
  var_dump($x->opt28);
  var_dump($x->opt29);
  var_dump($x->opt30);
  var_dump($x->opt31);

  var_dump(A::$s1);
  var_dump(A::$s2);
  var_dump(A::$s3);
  var_dump(A::$s4);
  var_dump(A::$s5);
  var_dump(A::$s6);
  var_dump(A::$s7);
  var_dump(A::$s8);
  var_dump(A::$s9);
  var_dump(A::$s10);
  var_dump(A::$s11);
  var_dump(A::$s12);
  var_dump(A::$s13);
  var_dump(A::$s14);
  var_dump(A::$s15);
  var_dump(A::$s16);
  var_dump(A::$s17);
  var_dump(A::$s18);
  var_dump(A::$s19);
  var_dump(A::$s20);
  var_dump(A::$s21);
  var_dump(A::$s22);
  var_dump(A::$s23);
  var_dump(A::$s24);
  var_dump(A::$s25);
  var_dump(A::$s26);
  var_dump(A::$s27);
  var_dump(A::$s28);
  var_dump(A::$s29);
  var_dump(A::$s30);
  var_dump(A::$s31);

  var_dump(A::$sopt1);
  var_dump(A::$sopt2);
  var_dump(A::$sopt3);
  var_dump(A::$sopt4);
  var_dump(A::$sopt5);
  var_dump(A::$sopt6);
  var_dump(A::$sopt7);
  var_dump(A::$sopt8);
  var_dump(A::$sopt9);
  var_dump(A::$sopt10);
  var_dump(A::$sopt11);
  var_dump(A::$sopt12);
  var_dump(A::$sopt13);
  var_dump(A::$sopt14);
  var_dump(A::$sopt15);
  var_dump(A::$sopt16);
  var_dump(A::$sopt17);
  var_dump(A::$sopt18);
  var_dump(A::$sopt19);
  var_dump(A::$sopt20);
  var_dump(A::$sopt21);
  var_dump(A::$sopt22);
  var_dump(A::$sopt23);
  var_dump(A::$sopt24);
  var_dump(A::$sopt25);
  var_dump(A::$sopt26);
  var_dump(A::$sopt27);
  var_dump(A::$sopt28);
  var_dump(A::$sopt29);
  var_dump(A::$sopt30);
  var_dump(A::$sopt31);
}
<<__EntryPoint>>
function main_entry(): void {
  if (__hhvm_intrinsics\launder_value(true)) {
    include 'redefine1.inc';
  } else {
    include 'redefine2.inc';
  }

  test(new A());
}
