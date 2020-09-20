<?hh

trait ATrait {
  public static function get_class_name() {
    return __CLASS__;
  }

  public function get_class_name_obj() {
    return __CLASS__;
  }
}

trait Indirect {
    use ATrait;
}

class SomeClass {
   use ATrait;
}

class UsingIndirect {
    use Indirect;
}
<<__EntryPoint>> function main(): void {
$r = SomeClass::get_class_name();
var_dump($r);

$o = new SomeClass();
$r = $o->get_class_name_obj();
var_dump($r);

$r = UsingIndirect::get_class_name();
var_dump($r);

$o = new UsingIndirect();
$r = $o->get_class_name_obj();
var_dump($r);
}
