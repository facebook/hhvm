<?hh

function handle_error($errno, $errstr, ...) :mixed{
  echo "handle_error(): [$errno]: $errstr\n";
  return true;
}

class K   { const A = 0; const B = 1; }
class Foo { static function bar() :mixed{} }

function P(bool $b) :mixed{ return $b ? "True\n" : "False\n"; }
function LV($x)     :mixed{ return __hhvm_intrinsics\launder_value($x); }

function is_as_static() :mixed{
  $m = Foo::bar<>;

  echo '$m is arraylike: '      .P($m is AnyArray);
  echo '$m is shape(str,str): ' .P($m is shape(K::A => string, K::B => string));
  echo '$m is shape(...): '     .P($m is shape(...));
  echo '$m is Traversable: '    .P($m is Traversable);
  echo '$m is Container: '      .P($m is Container);
  echo '$m is (string,string): '.P($m is (string, string));
  echo '$m[0] is string: '      .P($m[0] is string);
  echo '$m[1] is string: '      .P($m[1] is string);

  try { $m    as AnyArray;       } catch (Exception $e) { echo "Passed!\n"; }
  try { $m    as shape(K::A => string, K::B => string);
                                  } catch (Exception $e) { echo "shape!\n"; }
  try { $m    as shape(...);      } catch (Exception $e) { echo "shape!\n"; }
  try { $m    as Traversable;     } catch (Exception $e) { echo "Passed!\n"; }
  try { $m    as Container;       } catch (Exception $e) { echo "Passed!\n"; }
  try { $m    as (string,string); } catch (Exception $e) { echo "Passed!\n"; }
  try { $m[0] as string;          } catch (Exception $e) { echo "Passed!\n"; }
  try { $m[1] as string;          } catch (Exception $e) { echo "Passed!\n"; }

  try { var_dump(varray($m));     } catch (Exception $e) { echo "Passed\n"; }
}

function is_as_dynamic() :mixed{
  $m = LV(Foo::bar<>);

  echo '$m is arraylike: '      .P($m is AnyArray);
  echo '$m is shape(str,str): ' .P($m is shape(K::A => string, K::B => string));
  echo '$m is shape(...): '     .P($m is shape(...));
  echo '$m is Traversable: '    .P($m is Traversable);
  echo '$m is Container: '      .P($m is Container);
  echo '$m is (string,string): '.P($m is (string, string));
  echo '$m[0] is string: '      .P($m[0] is string);
  echo '$m[1] is string: '      .P($m[1] is string);

  try { $m    as AnyArray;       } catch (Exception $e) { echo "Passed!\n"; }
  try { $m    as shape(K::A => string, K::B => string);
                                  } catch (Exception $e) { echo "shape!\n"; }
  try { $m    as shape(...);      } catch (Exception $e) { echo "shape!\n"; }
  try { $m    as Traversable;     } catch (Exception $e) { echo "Passed!\n"; }
  try { $m    as Container;       } catch (Exception $e) { echo "Passed!\n"; }
  try { $m    as (string,string); } catch (Exception $e) { echo "Passed!\n"; }
  try { $m[0] as string;          } catch (Exception $e) { echo "Passed!\n"; }
  try { $m[1] as string;          } catch (Exception $e) { echo "Passed!\n"; }

  try { var_dump(varray($m));     } catch (Exception $e) { echo "Passed\n"; }
}

function is_as_shuffle_static() :mixed{
  $m = Foo::bar<>;

  if (is_array($m)) {
    $x = varray($m);
    echo '$m === varray($m): '.P($m === $x);

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

function is_as_shuffle_dynamic() :mixed{
  $m = LV(Foo::bar<>);

  if (is_array($m)) {
    $x = varray($m);
    echo '$m === varray($m): '.P($m === $x);

    if ($m is AnyArray) {
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
function main() :mixed{
  set_error_handler(handle_error<>);

  is_as_static();          is_as_static();          is_as_static();
  is_as_dynamic();         is_as_dynamic();         is_as_dynamic();
  is_as_shuffle_static();  is_as_shuffle_static();  is_as_shuffle_static();
  is_as_shuffle_dynamic(); is_as_shuffle_dynamic(); is_as_shuffle_dynamic();
}
