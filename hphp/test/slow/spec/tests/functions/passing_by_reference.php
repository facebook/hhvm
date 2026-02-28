<?hh

// A useful pass-by-reference example; swap the values of two variables

function swap(inout $p1, inout $p2)
:mixed{
   $temp = $p1;
   $p1 = $p2;
   $p2 = $temp;
}

///*
// a simple example of passing by reference

function f(inout $p)
:mixed{
   echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
    echo "f In:  \$p: $p\n";
    $p = 200;       // actual argument's value changed
    echo "f Out: \$p: $p\n";
}

//*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $a1 = 1.23e27;
  $a2 = vec[10,TRUE,NULL];
  var_dump($a1);
  var_dump($a2);
  swap(inout $a1, inout $a2);
  var_dump($a1);
  var_dump($a2);

  // pass a variable by reference; f changes its value

  $a = 10;
  var_dump($a);
  f(inout $a);   // change $a from 10 to 200
  var_dump($a);
  // f(inout $a);  // PHP5 32/62, Fatal error: Call-time pass-by-reference has been removed
           // HHVM accepts the inout  as being redundant
           // The php.net on-line help states: "As of PHP 5.3.0, you will get a warning
           // saying that "call-time pass-by-reference" is deprecated when you use inout  in
           // foo(inout $a);. And as of PHP 5.4.0, call-time pass-by-reference was removed,
           // so using it will raise a fatal error."
  var_dump($a);

  try { f(); } catch (Exception $e) { var_dump($e->getMessage()); }

  // pass a variable by reference; f changes its value
}
