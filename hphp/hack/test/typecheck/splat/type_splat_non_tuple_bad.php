<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function test1<T>(...T $x):void { }
function test2(...int $x):void { }
function test3(...vec<int> $x):void { }

class C {
  public static function test1<T>(...T $x):void { }
  public static function test2(...int $x):void { }
  public static function test3(...vec<int> $x):void { }

  public static function inst1<T>(...T $x):void { }
  public static function inst2(...int $x):void { }
  public static function inst3(...vec<int> $x):void { }

}
