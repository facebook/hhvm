<?hh

<<__EntryPoint>>
function entrypoint_anonymous_func_002(): void {

  $test = $v ==> $v;

  $GLOBALS['arr'] = varray[() ==> $GLOBALS['arr'], 2];

  var_dump($GLOBALS['arr'][$test(1)]);
  var_dump($GLOBALS['arr'][$test(0)]() == $GLOBALS['arr']);
}
