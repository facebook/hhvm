<?hh

// HHBBC marked CastInt as non-throwing for everything but objects, so DCE
// dropped the cast whenever its result was unused. Every cast below is dead
// on purpose: that is what makes the op removable, and the only thing left
// to keep it alive is the throw.
//
// Class and lazy class pointers raise a notice on the same path but are not
// reachable from Hack source, so they are not covered here.

class CastIntC {
  public static function sm(): void {}
  public static function smr<reify T>(): void {}
}
enum class CastIntE: mixed { int A = 1; }
function castint_fn(): void {}
function castint_fnr<reify T>(): void {}

function castint_func(): void     { $v = castint_fn<>;       $unused = (int)$v; }
function castint_rfunc(): void    { $v = castint_fnr<int>;   $unused = (int)$v; }
function castint_clsmeth(): void  { $v = CastIntC::sm<>;     $unused = (int)$v; }
function castint_rclsmeth(): void { $v = CastIntC::smr<int>; $unused = (int)$v; }
function castint_label(): void    { $v = CastIntE#A;         $unused = (int)$v; }
function castint_object(): void   { $v = new CastIntC();     $unused = (int)$v; }
function castint_string(): void   { $v = 'abc';              $unused = (int)$v; }
function castint_vec(): void      { $v = vec[1, 2];          $unused = (int)$v; }

<<__EntryPoint>>
function main_cast_to_int_throwing_types(): void {
  $cases = dict[
    'func' => castint_func<>,
    'rfunc' => castint_rfunc<>,
    'clsmeth' => castint_clsmeth<>,
    'rclsmeth' => castint_rclsmeth<>,
    'label' => castint_label<>,
    'object' => castint_object<>,
    'string' => castint_string<>,
    'vec' => castint_vec<>,
  ];
  foreach ($cases as $name => $f) {
    try {
      $f();
      echo $name, ": no exception\n";
    } catch (\Throwable $e) {
      echo $name, ": ", \get_class($e), "\n";
    }
  }
}
