<?hh

abstract final class ObjectMethod {
  public static $trace;
}

class W {
  <<__DynamicallyCallable>> function f($a) :mixed{
    ObjectMethod::$trace .= $a;
    return $a;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $w = new W();
  ObjectMethod::$trace = "a";
  call_user_func_array(vec[$w, 'f'], vec["b"]);
  echo ObjectMethod::$trace . "\n";
  call_user_func_array(vec[$w, 'f'], vec["c"]);
  echo ObjectMethod::$trace . "\n";
}
