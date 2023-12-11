<?hh

function LV(mixed $x): mixed { return __hhvm_intrinsics\launder_value($x); }

function handle_error($_errno, $msg, ...) :mixed{
  $matches = null;
  $pat1 =
    "/(Static p|P)roperty '(Props::[a-z_]+)' declared as type ".
    "(\??Foo|\??HH\\\\[a-zA-Z_]+), ([a-z]+) assigned/";
  if (preg_match_with_matches($pat1, $msg, inout $matches)) {
    echo "[TYPE VIOLATION] {$matches[2]} (wanted: {$matches[3]}, ".
         "got: {$matches[4]})\n";
    throw new Exception;
  }
  return false;
}

class Foo { static function bar() :mixed{} }

<<__NEVER_INLINE>>
function C(mixed $m): void { echo gettype($m)."\n"; }

class Props {
  public  static Foo          $pub_sprop_foo_static;
  public  static Foo          $pub_sprop_foo_dynamic;
  public  static Traversable  $pub_sprop_trav_static         = vec[];
  public  static Traversable  $pub_sprop_trav_dynamic        = vec[];
  public  static varray       $pub_sprop_varray_static       = vec[];
  public  static varray       $pub_sprop_varray_dynamic      = vec[];
  public  static ?Foo         $pub_sprop_opt_foo_static      = null;
  public  static ?Foo         $pub_sprop_opt_foo_dynamic     = null;
  public  static ?Traversable $pub_sprop_opt_trav_static     = null;
  public  static ?Traversable $pub_sprop_opt_trav_dynamic    = null;
  public  static ?varray      $pub_sprop_opt_varray_static   = null;
  public  static ?varray      $pub_sprop_opt_varray_dynamic  = null;

  private static Foo          $priv_sprop_foo_static;
  private static Foo          $priv_sprop_foo_dynamic;
  private static Traversable  $priv_sprop_trav_static        = vec[];
  private static Traversable  $priv_sprop_trav_dynamic       = vec[];
  private static varray       $priv_sprop_varray_static      = vec[];
  private static varray       $priv_sprop_varray_dynamic     = vec[];
  private static ?Foo         $priv_sprop_opt_foo_static     = null;
  private static ?Foo         $priv_sprop_opt_foo_dynamic    = null;
  private static ?Traversable $priv_sprop_opt_trav_static    = null;
  private static ?Traversable $priv_sprop_opt_trav_dynamic   = null;
  private static ?varray      $priv_sprop_opt_varray_static  = null;
  private static ?varray      $priv_sprop_opt_varray_dynamic = null;

  public         Foo          $pub_prop_foo_static;
  public         Foo          $pub_prop_foo_dynamic;
  public         Traversable  $pub_prop_trav_static          = vec[];
  public         Traversable  $pub_prop_trav_dynamic         = vec[];
  public         varray       $pub_prop_varray_static        = vec[];
  public         varray       $pub_prop_varray_dynamic       = vec[];
  public         ?Foo         $pub_prop_opt_foo_static       = null;
  public         ?Foo         $pub_prop_opt_foo_dynamic      = null;
  public         ?Traversable $pub_prop_opt_trav_static      = null;
  public         ?Traversable $pub_prop_opt_trav_dynamic     = null;
  public         ?varray      $pub_prop_opt_varray_static    = null;
  public         ?varray      $pub_prop_opt_varray_dynamic   = null;

  private        Foo          $priv_prop_foo_static;
  private        Foo          $priv_prop_foo_dynamic;
  private        Traversable  $priv_prop_trav_static         = vec[];
  private        Traversable  $priv_prop_trav_dynamic        = vec[];
  private        varray       $priv_prop_varray_static       = vec[];
  private        varray       $priv_prop_varray_dynamic      = vec[];
  private        ?Foo         $priv_prop_opt_foo_static      = null;
  private        ?Foo         $priv_prop_opt_foo_dynamic     = null;
  private        ?Traversable $priv_prop_opt_trav_static     = null;
  private        ?Traversable $priv_prop_opt_trav_dynamic    = null;
  private        ?varray      $priv_prop_opt_varray_static   = null;
  private        ?varray      $priv_prop_opt_varray_dynamic  = null;

  public static function assign_priv_sprop_static() :mixed{
    $val = Foo::bar<>;
    try { Props::$priv_sprop_foo_static     = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_trav_static        = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_varray_static      = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_opt_foo_static = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_opt_trav_static    = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_opt_varray_static  = $val; } catch (Exception $_) {}
  }

  public static function assign_priv_sprop_dynamic1() :mixed{
    $val = LV(Foo::bar<>);
    try { Props::$priv_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_trav_dynamic        = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_varray_dynamic      = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_opt_trav_dynamic    = $val; } catch (Exception $_) {}
    try { Props::$priv_sprop_opt_varray_dynamic  = $val; } catch (Exception $_) {}
  }

  public static function assign_priv_sprop_dynamic2() :mixed{
    $val = LV(Foo::bar<>);
    $base = LV('Props');
    try { Props::$priv_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
    try { $base::$priv_sprop_trav_dynamic        = $val; } catch (Exception $_) {}
    try { $base::$priv_sprop_varray_dynamic      = $val; } catch (Exception $_) {}
    try { $base::$priv_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
    try { $base::$priv_sprop_opt_trav_dynamic    = $val; } catch (Exception $_) {}
    try { $base::$priv_sprop_opt_varray_dynamic  = $val; } catch (Exception $_) {}
  }

  public static function dump_priv_sprops() :mixed{
    $base = LV('Props');

    var_dump(Props::$priv_sprop_foo_static);
    var_dump(Props::$priv_sprop_trav_static);
    var_dump(Props::$priv_sprop_varray_static);
    var_dump(Props::$priv_sprop_opt_foo_static);
    var_dump(Props::$priv_sprop_opt_trav_static);
    var_dump(Props::$priv_sprop_opt_varray_static);

    var_dump($base::$priv_sprop_foo_dynamic);
    var_dump($base::$priv_sprop_trav_dynamic);
    var_dump($base::$priv_sprop_varray_dynamic);
    var_dump($base::$priv_sprop_opt_foo_dynamic);
    var_dump($base::$priv_sprop_opt_trav_dynamic);
    var_dump($base::$priv_sprop_opt_varray_dynamic);
  }

  public function assign_priv_prop_static() :mixed{
    $m = Foo::bar<>;

    try { $this->priv_prop_foo_static     = $m; } catch (Exception $_) {}
    try { $this->priv_prop_trav_static        = $m; } catch (Exception $_) {}
    try { $this->priv_prop_varray_static      = $m; } catch (Exception $_) {}
    try { $this->priv_prop_opt_foo_static = $m; } catch (Exception $_) {}
    try { $this->priv_prop_opt_trav_static    = $m; } catch (Exception $_) {}
    try { $this->priv_prop_opt_varray_static  = $m; } catch (Exception $_) {}
  }

  public function assign_priv_prop_dynamic1() :mixed{
    $m = LV(Foo::bar<>);

    try { $this->priv_prop_foo_dynamic     = $m; } catch (Exception $_) {}
    try { $this->priv_prop_trav_dynamic        = $m; } catch (Exception $_) {}
    try { $this->priv_prop_varray_dynamic      = $m; } catch (Exception $_) {}
    try { $this->priv_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
    try { $this->priv_prop_opt_trav_dynamic    = $m; } catch (Exception $_) {}
    try { $this->priv_prop_opt_varray_dynamic  = $m; } catch (Exception $_) {}

  }

  public function assign_priv_prop_dynamic2() :mixed{
    $m = LV(Foo::bar<>);

    try { LV($this)->priv_prop_foo_dynamic     = $m; } catch (Exception $_) {}
    try { LV($this)->priv_prop_trav_dynamic        = $m; } catch (Exception $_) {}
    try { LV($this)->priv_prop_varray_dynamic      = $m; } catch (Exception $_) {}
    try { LV($this)->priv_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
    try { LV($this)->priv_prop_opt_trav_dynamic    = $m; } catch (Exception $_) {}
    try { LV($this)->priv_prop_opt_varray_dynamic  = $m; } catch (Exception $_) {}
  }

  public function dump_priv_props() :mixed{
    var_dump($this->priv_prop_foo_static);
    var_dump($this->priv_prop_trav_static);
    var_dump($this->priv_prop_varray_static);
    var_dump($this->priv_prop_opt_foo_static);
    var_dump($this->priv_prop_opt_trav_static);
    var_dump($this->priv_prop_opt_varray_static);
    var_dump(LV($this)->priv_prop_foo_dynamic);
    var_dump(LV($this)->priv_prop_trav_dynamic);
    var_dump(LV($this)->priv_prop_varray_dynamic);
    var_dump(LV($this)->priv_prop_opt_foo_dynamic);
    var_dump(LV($this)->priv_prop_opt_trav_dynamic);
    var_dump(LV($this)->priv_prop_opt_varray_dynamic);
  }
}

function assign_pub_sprop_static() :mixed{
  $val = Foo::bar<>;

  try { Props::$pub_sprop_foo_static     = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_trav_static        = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_varray_static      = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_foo_static = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_trav_static    = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_varray_static  = $val; } catch (Exception $_) {}
}

function assign_pub_sprop_dynamic1() :mixed{
  $val = LV(Foo::bar<>);

  try { Props::$pub_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_trav_dynamic        = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_varray_dynamic      = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_trav_dynamic    = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_varray_dynamic  = $val; } catch (Exception $_) {}
}

function assign_pub_sprop_dynamic2() :mixed{
  $val = LV(Foo::bar<>);
  $base = LV('Props');

  try { Props::$pub_sprop_foo_dynamic     = $val; } catch (Exception $_) {}
  try { $base::$pub_sprop_trav_dynamic        = $val; } catch (Exception $_) {}
  try { $base::$pub_sprop_varray_dynamic      = $val; } catch (Exception $_) {}
  try { Props::$pub_sprop_opt_foo_dynamic = $val; } catch (Exception $_) {}
  try { $base::$pub_sprop_opt_trav_dynamic    = $val; } catch (Exception $_) {}
  try { $base::$pub_sprop_opt_varray_dynamic  = $val; } catch (Exception $_) {}
}

function dump_pub_sprops() :mixed{
  $base = LV('Props');

  var_dump(Props::$pub_sprop_foo_static);
  var_dump(Props::$pub_sprop_trav_static);
  var_dump(Props::$pub_sprop_varray_static);
  var_dump(Props::$pub_sprop_opt_foo_static);
  var_dump(Props::$pub_sprop_opt_trav_static);
  var_dump(Props::$pub_sprop_opt_varray_static);

  var_dump($base::$pub_sprop_foo_dynamic);
  var_dump($base::$pub_sprop_trav_dynamic);
  var_dump($base::$pub_sprop_varray_dynamic);
  var_dump($base::$pub_sprop_opt_foo_dynamic);
  var_dump($base::$pub_sprop_opt_trav_dynamic);
  var_dump($base::$pub_sprop_opt_varray_dynamic);
}

function assign_pub_prop_static(Props $p) :mixed{
  $m = Foo::bar<>;

  try { $p->pub_prop_foo_static     = $m; } catch (Exception $_) {}
  try { $p->pub_prop_trav_static        = $m; } catch (Exception $_) {}
  try { $p->pub_prop_varray_static      = $m; } catch (Exception $_) {}
  try { $p->pub_prop_opt_foo_static = $m; } catch (Exception $_) {}
  try { $p->pub_prop_opt_trav_static    = $m; } catch (Exception $_) {}
  try { $p->pub_prop_opt_varray_static  = $m; } catch (Exception $_) {}
}

function assign_pub_prop_dynamic1(Props $p) :mixed{
  $m = LV(Foo::bar<>);

  try { $p->pub_prop_foo_dynamic     = $m; } catch (Exception $_) {}
  try { $p->pub_prop_trav_dynamic        = $m; } catch (Exception $_) {}
  try { $p->pub_prop_varray_dynamic      = $m; } catch (Exception $_) {}
  try { $p->pub_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
  try { $p->pub_prop_opt_trav_dynamic    = $m; } catch (Exception $_) {}
  try { $p->pub_prop_opt_varray_dynamic  = $m; } catch (Exception $_) {}

}

function assign_pub_prop_dynamic2(mixed $p) :mixed{
  $m = LV(Foo::bar<>);

  try { LV($p)->pub_prop_foo_dynamic     = $m; } catch (Exception $_) {}
  try { LV($p)->pub_prop_trav_dynamic        = $m; } catch (Exception $_) {}
  try { LV($p)->pub_prop_varray_dynamic      = $m; } catch (Exception $_) {}
  try { LV($p)->pub_prop_opt_foo_dynamic = $m; } catch (Exception $_) {}
  try { LV($p)->pub_prop_opt_trav_dynamic    = $m; } catch (Exception $_) {}
  try { LV($p)->pub_prop_opt_varray_dynamic  = $m; } catch (Exception $_) {}
}

function dump_pub_props(Props $p) :mixed{
  var_dump($p->pub_prop_foo_static);
  var_dump($p->pub_prop_trav_static);
  var_dump($p->pub_prop_varray_static);
  var_dump($p->pub_prop_opt_foo_static);
  var_dump($p->pub_prop_opt_trav_static);
  var_dump($p->pub_prop_opt_varray_static);
  var_dump(LV($p)->pub_prop_foo_dynamic);
  var_dump(LV($p)->pub_prop_trav_dynamic);
  var_dump(LV($p)->pub_prop_varray_dynamic);
  var_dump(LV($p)->pub_prop_opt_foo_dynamic);
  var_dump(LV($p)->pub_prop_opt_trav_dynamic);
  var_dump(LV($p)->pub_prop_opt_varray_dynamic);
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);

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
