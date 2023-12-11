<?hh
const ONE = 1;
const TWO = 1;

class Foo {
  const ONE = 1;
  const TWO = 1;

  public static $mapWithConst = dict[self::ONE => 'one', self::TWO => 'two',];

  public static $mapWithConst1 = dict[1 => 'one', self::TWO => 'two',];
  public static $mapWithConst2 = dict[self::ONE => 'one', 1 => 'two',];

  public static $mapWithoutConst = dict[1 => 'one', 1 => 'two',];
}
<<__EntryPoint>> function main(): void {
$mapWithConst = dict[1 => 'one', 1 => 'two',];

$mapWithoutConst = dict[Foo::ONE => 'one', Foo::TWO => 'two',];
$mapWithoutConst0 = dict[1 => 'one', 1 => 'two',];
$mapWithoutConst1 = dict[ONE => 'one', 1 => 'two',];
$mapWithoutConst2 = dict[1 => 'one', TWO => 'two',];
$mapWithoutConst3 = dict[ONE => 'one', TWO => 'two',];

var_dump(Foo::$mapWithConst[1]);
var_dump(Foo::$mapWithConst1[1]);
var_dump(Foo::$mapWithConst2[1]);
var_dump(Foo::$mapWithoutConst[1]);
var_dump($mapWithConst[1]);
var_dump($mapWithoutConst[1]);
var_dump($mapWithoutConst0[1]);
var_dump($mapWithoutConst1[1]);
var_dump($mapWithoutConst2[1]);
var_dump($mapWithoutConst3[1]);
}
