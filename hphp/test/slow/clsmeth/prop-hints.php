<?hh

function LV(mixed $x): mixed { return __hhvm_intrinsics\launder_value($x); }

function handle_error($_errno, $msg, ...) {
  $matches = null;
  $pat1 =
    "/(Static p|P)roperty '(Props::[a-z_]+)' declared as type ".
    "(\??Foo), ([a-z]+) assigned/";
  if (preg_match_with_matches($pat1, $msg, inout $matches)) {
    echo "[TYPE VIOLATION] {$matches[2]} (wanted: {$matches[3]}, ".
         "got: {$matches[4]})\n";
    throw new Exception;
  }
  return false;
}

class Foo { static function bar() {} }

<<__NEVER_INLINE>>
function C(mixed $m): void { echo gettype($m)."\n"; }

class Props {
  public  static Foo          $pub_sprop_foo_static;
  public  static Foo          $pub_sprop_foo_dynamic;
  public  static Traversable  $pub_sprop_trav_static         = varray[];
  public  static Traversable  $pub_sprop_trav_dynamic        = varray[];
  public  static array        $pub_sprop_array_static        = varray[];
  public  static array        $pub_sprop_array_dynamic       = varray[];
  public  static varray       $pub_sprop_varray_static       = varray[];
  public  static varray       $pub_sprop_varray_dynamic      = varray[];
  public  static darray       $pub_sprop_darray_static       = darray[];
  public  static darray       $pub_sprop_darray_dynamic      = darray[];
  public  static ?Foo         $pub_sprop_opt_foo_static      = null;
  public  static ?Foo         $pub_sprop_opt_foo_dynamic     = null;
  public  static ?Traversable $pub_sprop_opt_trav_static     = null;
  public  static ?Traversable $pub_sprop_opt_trav_dynamic    = null;
  public  static ?array       $pub_sprop_opt_array_static    = null;
  public  static ?array       $pub_sprop_opt_array_dynamic   = null;
  public  static ?varray      $pub_sprop_opt_varray_static   = null;
  public  static ?varray      $pub_sprop_opt_varray_dynamic  = null;
  public  static ?darray      $pub_sprop_opt_darray_static   = null;
  public  static ?darray      $pub_sprop_opt_darray_dynamic  = null;

  private static Foo          $priv_sprop_foo_static;
  private static Foo          $priv_sprop_foo_dynamic;
  private static Traversable  $priv_sprop_trav_static        = varray[];
  private static Traversable  $priv_sprop_trav_dynamic       = varray[];
  private static array        $priv_sprop_array_static       = varray[];
  private static array        $priv_sprop_array_dynamic      = varray[];
  private static varray       $priv_sprop_varray_static      = varray[];
  private static varray       $priv_sprop_varray_dynamic     = varray[];
  private static darray       $priv_sprop_darray_static      = darray[];
  private static darray       $priv_sprop_darray_dynamic     = darray[];
  private static ?Foo         $priv_sprop_opt_foo_static     = null;
  private static ?Foo         $priv_sprop_opt_foo_dynamic    = null;
  private static ?Traversable $priv_sprop_opt_trav_static    = null;
  private static ?Traversable $priv_sprop_opt_trav_dynamic   = null;
  private static ?array       $priv_sprop_opt_array_static   = null;
  private static ?array       $priv_sprop_opt_array_dynamic  = null;
  private static ?darray      $priv_sprop_opt_varray_static  = null;
  private static ?varray      $priv_sprop_opt_varray_dynamic = null;
  private static ?varray      $priv_sprop_opt_darray_static  = null;
  private static ?darray      $priv_sprop_opt_darray_dynamic = null;

  public         Foo          $pub_prop_foo_static;
  public         Foo          $pub_prop_foo_dynamic;
  public         Traversable  $pub_prop_trav_static          = varray[];
  public         Traversable  $pub_prop_trav_dynamic         = varray[];
  public         array        $pub_prop_array_static         = varray[];
  public         array        $pub_prop_array_dynamic        = varray[];
  public         varray       $pub_prop_varray_static        = varray[];
  public         varray       $pub_prop_varray_dynamic       = varray[];
  public         darray       $pub_prop_darray_static        = darray[];
  public         darray       $pub_prop_darray_dynamic       = darray[];
  public         ?Foo         $pub_prop_opt_foo_static       = null;
  public         ?Foo         $pub_prop_opt_foo_dynamic      = null;
  public         ?Traversable $pub_prop_opt_trav_static      = null;
  public         ?Traversable $pub_prop_opt_trav_dynamic     = null;
  public         ?array       $pub_prop_opt_array_static     = null;
  public         ?array       $pub_prop_opt_array_dynamic    = null;
  public         ?varray      $pub_prop_opt_varray_static    = null;
  public         ?varray      $pub_prop_opt_varray_dynamic   = null;
  public         ?darray      $pub_prop_opt_darray_static    = null;
  public         ?darray      $pub_prop_opt_darray_dynamic   = null;

  private        Foo          $priv_prop_foo_static;
  private        Foo          $priv_prop_foo_dynamic;
  private        Traversable  $priv_prop_trav_static         = varray[];
  private        Traversable  $priv_prop_trav_dynamic        = varray[];
  private        array        $priv_prop_array_static        = varray[];
  private        array        $priv_prop_array_dynamic       = varray[];
  private        varray       $priv_prop_varray_static       = varray[];
  private        varray       $priv_prop_varray_dynamic      = varray[];
  private        darray       $priv_prop_darray_static       = darray[];
  private        darray       $priv_prop_darray_dynamic      = darray[];
  private        ?Foo         $priv_prop_opt_foo_static      = null;
  private        ?Foo         $priv_prop_opt_foo_dynamic     = null;
  private        ?Traversable $priv_prop_opt_trav_static     = null;
  private        ?Traversable $priv_prop_opt_trav_dynamic    = null;
  private        ?array       $priv_prop_opt_array_static    = null;
  private        ?array       $priv_prop_opt_array_dynamic   = null;
  private        ?varray      $priv_prop_opt_varray_static   = null;
  private        ?varray      $priv_prop_opt_varray_dynamic  = null;
  private        ?darray      $priv_prop_opt_darray_static   = null;
  private        ?darray      $priv_prop_opt_darray_dynamic  = null;

  public static function assign_priv_sprop_static() {
    $val = class_meth(Foo::class, 'bar');
    try { Props::$priv_sprop_foo_static     = $val; } catch (Exception $_) {}
    C(Props::$priv_sprop_trav_static        = $val);
    C(Props::$priv_sprop_array_static       = $val);
    C(Props::$priv_sprop_varray_static      = $val);
    C(Props::$priv_sprop_darray_static      = $val);
    try { Props::$priv_sprop_opt_foo_static = $val; } catch (Exception $_) {}
    C(Props::$priv_sprop_opt_trav_static    = $val);
    C(Props::$priv_sprop_opt_array_static   = $val);
    C(Props::$priv_sprop_opt_varray_static  = $val);
    C(Props::$priv_sprop_opt_darray_static  = $val);
  }

  public static function assign_priv_sprop_dynamic1() {
    $val = LV(class_meth(Foo::class, 'bar'));
    try { Props::$priv_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
    C(Props::$priv_sprop_trav_dynamic        = $val);
    C(Props::$priv_sprop_array_dynamic       = $val);
    C(Props::$priv_sprop_varray_dynamic      = $val);
    C(Props::$priv_sprop_darray_dynamic      = $val);
    try { Props::$priv_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
    C(Props::$priv_sprop_opt_trav_dynamic    = $val);
    C(Props::$priv_sprop_opt_array_dynamic   = $val);
    C(Props::$priv_sprop_opt_varray_dynamic  = $val);
    C(Props::$priv_sprop_opt_darray_dynamic  = $val);
  }

  public static function assign_priv_sprop_dynamic2() {
    $val = LV(class_meth(Foo::class, 'bar'));
    $base = LV('Props');
    try { Props::$priv_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
    C($base::$priv_sprop_trav_dynamic        = $val);
    C($base::$priv_sprop_array_dynamic       = $val);
    C($base::$priv_sprop_varray_dynamic      = $val);
    C($base::$priv_sprop_darray_dynamic      = $val);
    try { $base::$priv_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
    C($base::$priv_sprop_opt_trav_dynamic    = $val);
    C($base::$priv_sprop_opt_array_dynamic   = $val);
    C($base::$priv_sprop_opt_varray_dynamic  = $val);
    C($base::$priv_sprop_opt_darray_dynamic  = $val);
  }

  public static function dump_priv_sprops() {
    $base = LV('Props');

    var_dump(Props::$priv_sprop_foo_static);
    var_dump(Props::$priv_sprop_trav_static);
    var_dump(Props::$priv_sprop_array_static);
    var_dump(Props::$priv_sprop_varray_static);
    var_dump(Props::$priv_sprop_darray_static);
    var_dump(Props::$priv_sprop_opt_foo_static);
    var_dump(Props::$priv_sprop_opt_trav_static);
    var_dump(Props::$priv_sprop_opt_array_static);
    var_dump(Props::$priv_sprop_opt_varray_static);
    var_dump(Props::$priv_sprop_opt_darray_static);

    var_dump($base::$priv_sprop_foo_dynamic);
    var_dump($base::$priv_sprop_trav_dynamic);
    var_dump($base::$priv_sprop_array_dynamic);
    var_dump($base::$priv_sprop_varray_dynamic);
    var_dump($base::$priv_sprop_darray_dynamic);
    var_dump($base::$priv_sprop_opt_foo_dynamic);
    var_dump($base::$priv_sprop_opt_trav_dynamic);
    var_dump($base::$priv_sprop_opt_array_dynamic);
    var_dump($base::$priv_sprop_opt_varray_dynamic);
    var_dump($base::$priv_sprop_opt_darray_dynamic);
  }

  public function assign_priv_prop_static() {
    $m = class_meth(Foo::class, 'bar');

    try { $this->priv_prop_foo_static     = $m; } catch (Exception $_) {}
    C($this->priv_prop_trav_static        = $m);
    C($this->priv_prop_array_static       = $m);
    C($this->priv_prop_varray_static      = $m);
    C($this->priv_prop_darray_static      = $m);
    try { $this->priv_prop_opt_foo_static = $m; } catch (Exception $_) {}
    C($this->priv_prop_opt_trav_static    = $m);
    C($this->priv_prop_opt_array_static   = $m);
    C($this->priv_prop_opt_varray_static  = $m);
    C($this->priv_prop_opt_darray_static  = $m);
  }

  public function assign_priv_prop_dynamic1() {
    $m = LV(class_meth(Foo::class, 'bar'));

    try { $this->priv_prop_foo_dynamic     = $m; } catch (Exception $_) {}
    C($this->priv_prop_trav_dynamic        = $m);
    C($this->priv_prop_array_dynamic       = $m);
    C($this->priv_prop_varray_dynamic      = $m);
    C($this->priv_prop_darray_dynamic      = $m);
    try { $this->priv_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
    C($this->priv_prop_opt_trav_dynamic    = $m);
    C($this->priv_prop_opt_array_dynamic   = $m);
    C($this->priv_prop_opt_varray_dynamic  = $m);
    C($this->priv_prop_opt_darray_dynamic  = $m);

  }

  public function assign_priv_prop_dynamic2() {
    $m = LV(class_meth(Foo::class, 'bar'));

    try { LV($this)->priv_prop_foo_dynamic     = $m; } catch (Exception $_) {}
    C(LV($this)->priv_prop_trav_dynamic        = $m);
    C(LV($this)->priv_prop_array_dynamic       = $m);
    C(LV($this)->priv_prop_varray_dynamic      = $m);
    C(LV($this)->priv_prop_darray_dynamic      = $m);
    try { LV($this)->priv_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
    C(LV($this)->priv_prop_opt_trav_dynamic    = $m);
    C(LV($this)->priv_prop_opt_array_dynamic   = $m);
    C(LV($this)->priv_prop_opt_varray_dynamic  = $m);
    C(LV($this)->priv_prop_opt_darray_dynamic  = $m);
  }

  public function dump_priv_props() {
    var_dump($this->priv_prop_foo_static);
    var_dump($this->priv_prop_trav_static);
    var_dump($this->priv_prop_array_static);
    var_dump($this->priv_prop_varray_static);
    var_dump($this->priv_prop_darray_static);
    var_dump($this->priv_prop_opt_foo_static);
    var_dump($this->priv_prop_opt_trav_static);
    var_dump($this->priv_prop_opt_array_static);
    var_dump($this->priv_prop_opt_varray_static);
    var_dump($this->priv_prop_opt_darray_static);
    var_dump(LV($this)->priv_prop_foo_dynamic);
    var_dump(LV($this)->priv_prop_trav_dynamic);
    var_dump(LV($this)->priv_prop_array_dynamic);
    var_dump(LV($this)->priv_prop_varray_dynamic);
    var_dump(LV($this)->priv_prop_darray_dynamic);
    var_dump(LV($this)->priv_prop_opt_foo_dynamic);
    var_dump(LV($this)->priv_prop_opt_trav_dynamic);
    var_dump(LV($this)->priv_prop_opt_array_dynamic);
    var_dump(LV($this)->priv_prop_opt_varray_dynamic);
    var_dump(LV($this)->priv_prop_opt_darray_dynamic);
  }
}

function assign_pub_sprop_static() {
  $val = class_meth(Foo::class, 'bar');

  try { Props::$pub_sprop_foo_static     = $val; } catch (Exception $_) {}
  C(Props::$pub_sprop_trav_static        = $val);
  C(Props::$pub_sprop_array_static       = $val);
  C(Props::$pub_sprop_varray_static      = $val);
  C(Props::$pub_sprop_darray_static      = $val);
  try { Props::$pub_sprop_opt_foo_static = $val; } catch (Exception $_) {}
  C(Props::$pub_sprop_opt_trav_static    = $val);
  C(Props::$pub_sprop_opt_array_static   = $val);
  C(Props::$pub_sprop_opt_varray_static  = $val);
  C(Props::$pub_sprop_opt_darray_static  = $val);
}

function assign_pub_sprop_dynamic1() {
  $val = LV(class_meth(Foo::class, 'bar'));

  try { Props::$pub_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
  C(Props::$pub_sprop_trav_dynamic        = $val);
  C(Props::$pub_sprop_array_dynamic       = $val);
  C(Props::$pub_sprop_varray_dynamic      = $val);
  C(Props::$pub_sprop_darray_dynamic      = $val);
  try { Props::$pub_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
  C(Props::$pub_sprop_opt_trav_dynamic    = $val);
  C(Props::$pub_sprop_opt_array_dynamic   = $val);
  C(Props::$pub_sprop_opt_varray_dynamic  = $val);
  C(Props::$pub_sprop_opt_darray_dynamic  = $val);
}

function assign_pub_sprop_dynamic2() {
  $val = LV(class_meth(Foo::class, 'bar'));
  $base = LV('Props');

  try { Props::$pub_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
  C($base::$pub_sprop_trav_dynamic        = $val);
  C($base::$pub_sprop_array_dynamic       = $val);
  C($base::$pub_sprop_varray_dynamic      = $val);
  C($base::$pub_sprop_darray_dynamic      = $val);
  try { Props::$pub_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
  C($base::$pub_sprop_opt_trav_dynamic    = $val);
  C($base::$pub_sprop_opt_array_dynamic   = $val);
  C($base::$pub_sprop_opt_varray_dynamic  = $val);
  C($base::$pub_sprop_opt_darray_dynamic  = $val);
}

function dump_pub_sprops() {
  $base = LV('Props');

  var_dump(Props::$pub_sprop_foo_static);
  var_dump(Props::$pub_sprop_trav_static);
  var_dump(Props::$pub_sprop_array_static);
  var_dump(Props::$pub_sprop_varray_static);
  var_dump(Props::$pub_sprop_darray_static);
  var_dump(Props::$pub_sprop_opt_foo_static);
  var_dump(Props::$pub_sprop_opt_trav_static);
  var_dump(Props::$pub_sprop_opt_array_static);
  var_dump(Props::$pub_sprop_opt_varray_static);
  var_dump(Props::$pub_sprop_opt_darray_static);

  var_dump($base::$pub_sprop_foo_dynamic);
  var_dump($base::$pub_sprop_trav_dynamic);
  var_dump($base::$pub_sprop_array_dynamic);
  var_dump($base::$pub_sprop_varray_dynamic);
  var_dump($base::$pub_sprop_darray_dynamic);
  var_dump($base::$pub_sprop_opt_foo_dynamic);
  var_dump($base::$pub_sprop_opt_trav_dynamic);
  var_dump($base::$pub_sprop_opt_array_dynamic);
  var_dump($base::$pub_sprop_opt_varray_dynamic);
  var_dump($base::$pub_sprop_opt_darray_dynamic);
}

function assign_pub_prop_static(Props $p) {
  $m = class_meth(Foo::class, 'bar');

  try { $p->pub_prop_foo_static     = $m; } catch (Exception $_) {}
  C($p->pub_prop_trav_static        = $m);
  C($p->pub_prop_array_static       = $m);
  C($p->pub_prop_varray_static      = $m);
  C($p->pub_prop_darray_static      = $m);
  try { $p->pub_prop_opt_foo_static = $m; } catch (Exception $_) {}
  C($p->pub_prop_opt_trav_static    = $m);
  C($p->pub_prop_opt_array_static   = $m);
  C($p->pub_prop_opt_varray_static  = $m);
  C($p->pub_prop_opt_darray_static  = $m);
}

function assign_pub_prop_dynamic1(Props $p) {
  $m = LV(class_meth(Foo::class, 'bar'));

  try { $p->pub_prop_foo_dynamic     = $m; } catch (Exception $_) {}
  C($p->pub_prop_trav_dynamic        = $m);
  C($p->pub_prop_array_dynamic       = $m);
  C($p->pub_prop_varray_dynamic      = $m);
  C($p->pub_prop_darray_dynamic      = $m);
  try { $p->pub_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
  C($p->pub_prop_opt_trav_dynamic    = $m);
  C($p->pub_prop_opt_array_dynamic   = $m);
  C($p->pub_prop_opt_varray_dynamic  = $m);
  C($p->pub_prop_opt_darray_dynamic  = $m);

}

function assign_pub_prop_dynamic2(mixed $p) {
  $m = LV(class_meth(Foo::class, 'bar'));

  try { LV($p)->pub_prop_foo_dynamic     = $m; } catch (Exception $_) {}
  C(LV($p)->pub_prop_trav_dynamic        = $m);
  C(LV($p)->pub_prop_array_dynamic       = $m);
  C(LV($p)->pub_prop_varray_dynamic      = $m);
  C(LV($p)->pub_prop_darray_dynamic      = $m);
  try { LV($p)->pub_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
  C(LV($p)->pub_prop_opt_trav_dynamic    = $m);
  C(LV($p)->pub_prop_opt_array_dynamic   = $m);
  C(LV($p)->pub_prop_opt_varray_dynamic  = $m);
  C(LV($p)->pub_prop_opt_darray_dynamic  = $m);
}

function dump_pub_props(Props $p) {
  var_dump($p->pub_prop_foo_static);
  var_dump($p->pub_prop_trav_static);
  var_dump($p->pub_prop_array_static);
  var_dump($p->pub_prop_varray_static);
  var_dump($p->pub_prop_darray_static);
  var_dump($p->pub_prop_opt_foo_static);
  var_dump($p->pub_prop_opt_trav_static);
  var_dump($p->pub_prop_opt_array_static);
  var_dump($p->pub_prop_opt_varray_static);
  var_dump($p->pub_prop_opt_darray_static);
  var_dump(LV($p)->pub_prop_foo_dynamic);
  var_dump(LV($p)->pub_prop_trav_dynamic);
  var_dump(LV($p)->pub_prop_array_dynamic);
  var_dump(LV($p)->pub_prop_varray_dynamic);
  var_dump(LV($p)->pub_prop_darray_dynamic);
  var_dump(LV($p)->pub_prop_opt_foo_dynamic);
  var_dump(LV($p)->pub_prop_opt_trav_dynamic);
  var_dump(LV($p)->pub_prop_opt_array_dynamic);
  var_dump(LV($p)->pub_prop_opt_varray_dynamic);
  var_dump(LV($p)->pub_prop_opt_darray_dynamic);
}

<<__EntryPoint>>
function main() {
  set_error_handler('handle_error');

  Props::assign_priv_sprop_static();   Props::assign_priv_sprop_static();
  Props::assign_priv_sprop_dynamic1(); Props::assign_priv_sprop_dynamic1();
  Props::assign_priv_sprop_dynamic2(); Props::assign_priv_sprop_dynamic2();

  assign_pub_sprop_static();   assign_pub_sprop_static();
  assign_pub_sprop_dynamic1(); assign_pub_sprop_dynamic1();
  assign_pub_sprop_dynamic2(); assign_pub_sprop_dynamic2();

  $obj = new Props;
  $obj->assign_priv_prop_static();   $obj->assign_priv_prop_static();
  $obj->assign_priv_prop_dynamic1(); $obj->assign_priv_prop_dynamic1();
  $obj->assign_priv_prop_dynamic2(); $obj->assign_priv_prop_dynamic2();

  assign_pub_prop_static($obj);   assign_pub_prop_static($obj);
  assign_pub_prop_dynamic1($obj); assign_pub_prop_dynamic1($obj);
  assign_pub_prop_dynamic2($obj); assign_pub_prop_dynamic2($obj);

  Props::dump_priv_sprops();
  dump_pub_sprops();
  $obj->dump_priv_props();
  dump_pub_props($obj);
}
