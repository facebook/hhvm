<?hh

class C {}

function f(): string {
  return __hhvm_intrinsics\launder_value(nameof C);
}

function g<reify T>(): void {
  $dlcls = T::class; // LazyCls
  var_dump("dlcls=".HH\class_to_classname($dlcls));
}

<<__EntryPoint>>
function main(): void {
  $cls = HH\classname_to_class(C::class); // Cls=C
  $dcls = HH\classname_to_class(__hhvm_intrinsics\launder_value(C::class)); // Cls
  $lcls = C::class; // LazyCls="C"
  $sstr = nameof C; // SStr="C"
  $dsstr = HH\class_to_classname($dcls); // SStr
  $str = f(); // Str
  $mixed = __hhvm_intrinsics\launder_value($str); // InitCell


  var_dump("cls=".HH\class_to_classname($cls));
  var_dump("dcls=".HH\class_to_classname($dcls));
  var_dump("lcls=".HH\class_to_classname($lcls));
  g<C>();
  var_dump("sstr=".HH\class_to_classname($sstr));
  var_dump("dsstr=".HH\class_to_classname($dsstr));
  var_dump("str=".HH\class_to_classname($str));
  var_dump("mixed=".HH\class_to_classname($mixed));

  // Obj=C -> Cls=C -> SStr="C" -> SStr="chain=C"
  var_dump("chain=".HH\class_to_classname(get_class(new C())));
  // redundant to $dcls case: Obj -> Cls -> SStr
  var_dump("dchain=".HH\class_to_classname(get_class(__hhvm_intrinsics\launder_value(new C()))));
}
