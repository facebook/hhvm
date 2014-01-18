<?hh

function x() {
  $str = "yo";
  $x = new ReflectionFunction(
    function () { static $something = 2; echo $str; }
  );

  // Differs from zend: we haven't initialized our static local yet,
  // so we'll say $something is NULL.
  var_dump($x->getStaticVariables());

  $fn = function () { static $something = 2; };
  $fn();
  $x = new ReflectionFunction($fn);
  var_dump($x->getStaticVariables()); // 'something' is 2; we've done
                                      // the StaticLocInit

  // Just a use var
  $y = 2;
  $x = new ReflectionFunction($x ==> $x + $y);
  var_dump($x->getStaticVariables());
}
x();
