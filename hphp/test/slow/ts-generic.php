<?hh

interface Iface {}

newtype FS<T> = string;
type S = FS<Iface>;
type S2 = S;

type R = FS<int>;
type R2 = R;

type Tx = FS;
type Ty<Tq> = FS;
type Tz<Tr,Ts> = FS;

type enumclassname<TClass, TValue> =
  classname<HH\GenericEnumClass<TClass, TValue>>;

interface I1 {}
interface I2 extends I1 {}

interface I3 {}
interface I4 extends I3 {}

newtype TA1<+Tr as arraykey, +Th> as Tr = Tr;
newtype TA2<+Tz> as TA1<int, Tz> = int;
newtype TA3<+Tx, +Ty, +Tz> as TA2<Tx> = TA2<Tx>;
type TA4<+Tq, +Tu> = TA3<Tq, Tu, I1>;

type TA5<+Tv> = TA4<Tv, I4>;
type TA6 = TA5<mixed>;
newtype TA7<+T as stdclass> as TA6 = TA6;

type Alpha<Ta> = int;
type Beta = Alpha<bool>;
type Gamma<Tb> = Beta;

type tn<Ta> = typename<Ta>;
type TEnum = shape('key' => tn<arraykey>, ...);

type Twhat<Tnone> = Tnone;
type Ttheheck = Twhat;

<<__EntryPoint>>
function main() {
  var_dump(\HH\type_structure_for_alias('S')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('S2')['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('R')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('R2')['typevars'] ?? null);

  $s1 = __hhvm_intrinsics\launder_value('S');
  $s2 = __hhvm_intrinsics\launder_value('S2');

  $r1 = __hhvm_intrinsics\launder_value('R');
  $r2 = __hhvm_intrinsics\launder_value('R2');

  var_dump(\HH\type_structure_for_alias($s1)['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($s2)['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias($r1)['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($r2)['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('\HH\Lib\Str\SprintfFormatString')['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('Tx')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('Ty')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('Tz')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('enumclassname')['typevars'] ?? null);

  $tx = __hhvm_intrinsics\launder_value('Tx');
  $ty = __hhvm_intrinsics\launder_value('Ty');
  $tz = __hhvm_intrinsics\launder_value('Tz');
  $ecn = __hhvm_intrinsics\launder_value('enumclassname');

  var_dump(\HH\type_structure_for_alias($tx)['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ty)['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($tz)['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ecn)['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('TA1')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TA2')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TA3')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TA4')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TA5')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TA6')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TA7')['typevars'] ?? null);

  $ta = __hhvm_intrinsics\launder_value('TA');
  var_dump(\HH\type_structure_for_alias($ta.'1')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ta.'2')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ta.'3')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ta.'4')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ta.'5')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ta.'6')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias($ta.'7')['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('Alpha')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('Beta')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('Gamma')['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('tn')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('TEnum')['fields']['key']['typevars'] ?? null);

  var_dump(\HH\type_structure_for_alias('Twhat')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('Ttheheck')['typevars'] ?? null);
  var_dump(\HH\type_structure_for_alias('Ttheheck') ?? null);
}
