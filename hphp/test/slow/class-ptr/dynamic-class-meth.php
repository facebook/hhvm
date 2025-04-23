<?hh

class C {
  <<__DynamicallyCallable>>
  public static function m(): void {
    echo "called m\n";
  }

  public static function n(): void {
    echo "called n\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $ss = nameof C;
  $s = __hhvm_intrinsics\launder_value($ss);
  $lc = C::class;
  $c = HH\classname_to_class($lc);

  HH\dynamic_class_meth($ss, 'm')();
  HH\dynamic_class_meth($s, 'm')();
  HH\dynamic_class_meth($lc, 'm')();
  HH\dynamic_class_meth($c, 'm')();

  HH\dynamic_class_meth($ss, 'n')();
  HH\dynamic_class_meth($s, 'n')();
  HH\dynamic_class_meth($lc, 'n')();
  HH\dynamic_class_meth($c, 'n')();
}
