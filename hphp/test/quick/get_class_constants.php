<?hh
// Never True

class Blah {
  const STRING_VAL = "zzz";
  const INT_VAL = 105;
  const FP_VAL = 3.14;
  const NULL_VAL = null;
}
<<__EntryPoint>> function main(): void {
var_dump(get_class_constants("Blah"));
}
