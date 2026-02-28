<?hh

class C {
  static public function func1() :mixed{
    return 1;
  }
}

/* class_meth used to be case insensitive in codegen. This used to work.
 * It should not anymore. */
<<__EntryPoint>>
function main() :mixed{
  $_ = hh\class_meth(C::class, 'func1');
}
