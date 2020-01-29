<?hh

class K   { const A = 0; const B = 1; };
class Foo { static function bar() {} }

function P(bool $b) { return $b ? "True\n" : "False\n"; }
function LV($x)     { return __hhvm_intrinsics\launder_value($x); }

function is_as_static() {
  $m = class_meth(Foo::class, 'bar');

  echo '$m is arraylike: '      .P($m is arraylike);
  echo '$m is shape(str,str): ' .P($m is shape(K::A => string, K::B => string));
  echo '$m is shape(...): '     .P($m is shape(...));
  echo '$m is Traversable: '    .P($m is Traversable);
  echo '$m is Container: '      .P($m is Container);
  echo '$m is (string,string): '.P($m is (string, string));
  echo '$m[0] is string: '      .P($m[0] is string);
  echo '$m[1] is string: '      .P($m[1] is string);

  try { $m    as arraylike;       } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as shape(K::A => string, K::B => string);
                                  } catch (exception $e) { echo "failed!\n"; }
  try { $m    as shape(...);      } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as Traversable;     } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as Container;       } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as (string,string); } catch (Exception $e) { echo "Failed!\n"; }
  try { $m[0] as string;          } catch (Exception $e) { echo "Failed!\n"; }
  try { $m[1] as string;          } catch (Exception $e) { echo "Failed!\n"; }

  var_dump(varray($m));
}

function is_as_dynamic() {
  $m = LV(class_meth(Foo::class, 'bar'));

  echo '$m is arraylike: '      .P($m is arraylike);
  echo '$m is shape(str,str): ' .P($m is shape(K::A => string, K::B => string));
  echo '$m is shape(...): '     .P($m is shape(...));
  echo '$m is Traversable: '    .P($m is Traversable);
  echo '$m is Container: '      .P($m is Container);
  echo '$m is (string,string): '.P($m is (string, string));
  echo '$m[0] is string: '      .P($m[0] is string);
  echo '$m[1] is string: '      .P($m[1] is string);

  try { $m    as arraylike;       } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as shape(K::A => string, K::B => string);
                                  } catch (exception $e) { echo "failed!\n"; }
  try { $m    as shape(...);      } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as Traversable;     } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as Container;       } catch (Exception $e) { echo "Failed!\n"; }
  try { $m    as (string,string); } catch (Exception $e) { echo "Failed!\n"; }
  try { $m[0] as string;          } catch (Exception $e) { echo "Failed!\n"; }
  try { $m[1] as string;          } catch (Exception $e) { echo "Failed!\n"; }

  var_dump(varray($m));
}

function is_as_shuffle_static() {
  $m = class_meth(Foo::class, 'bar');

  if (is_array($m)) {
    $x = $m as shape(...);
    echo '$m === ($m as shape(...)): '.P($m === $x);

    if ($m is Traversable) {
      $x = $m as Traversable;
      echo '$m === ($m as Traversable): '.P($m === $x);
    } else {
      echo "Failed \$m is Traversable!\n";
    }
  } else {
    echo "Failed \$m is array!\n";
  }
}

function is_as_shuffle_dynamic() {
  $m = LV(class_meth(Foo::class, 'bar'));

  if (is_array($m)) {
    $x = $m as shape(...);
    echo '$m === ($m as shape(...)): '.P($m === $x);

    if ($m is arraylike) {
      $x = $m as Traversable;
      echo '$m === ($m as arraylike): '.P($m === $x);
    } else {
      echo "Failed \$m is arraylike!\n";
    }
  } else {
    echo "Failed \$m is array!\n";
  }
}

<<__EntryPoint>>
function main() {
  is_as_static();          is_as_static();          is_as_static();
  is_as_dynamic();         is_as_dynamic();         is_as_dynamic();
  is_as_shuffle_static();  is_as_shuffle_static();  is_as_shuffle_static();
  is_as_shuffle_dynamic(); is_as_shuffle_dynamic(); is_as_shuffle_dynamic();
}
