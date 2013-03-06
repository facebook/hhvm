<?
// Never True
if (rand(100,101) == 2) {
    define("FOO", 1);
} else {
    define("FOO", 2);
}
class Blah {
  const DYNAMIC_VAL = FOO;
  const STRING_VAL = "zzz";
  const INT_VAL = 105;
  const FP_VAL = 3.14;
  const NULL_VAL = null;
}
var_dump(get_class_constants("Blah"));
