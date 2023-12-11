<?hh

class z {
  <<__DynamicallyCallable>> function minArgTest(
    $a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $a10, $a11=true, $a12 = true,
  ) :mixed{
    var_dump($a1);
    var_dump($a2);
    var_dump($a3);
    var_dump($a4);
    var_dump($a5);
    var_dump($a6);
    var_dump($a7);
    var_dump($a8);
    var_dump($a9);
    var_dump($a10);
    var_dump($a11);
    var_dump($a12);
  }
  <<__DynamicallyCallable>> function varArgsTest(...$args) :mixed{
    var_dump($args);
  }
  <<__DynamicallyCallable>> function varArgsTest2($a1, $a2, ...$more_args) :mixed{
    $args = vec[$a1, $a2];
    $args = array_merge($args, $more_args);
    var_dump($args);
  }
  function refTestHelper(inout $x) :mixed{
    $x *= 2;
  }
}
function refTest($q) :mixed{
  if (false) {
 $q = 1;
 }
  $x = 1;
  $q->refTestHelper(inout $x);
  var_dump($x);
}

<<__EntryPoint>>
function main_1209() :mixed{
$q = new z;
$f = 'minArgTest';
$q->minArgTest('one',2,3.333,4,5,6,7,8,9,10);
$q->minArgTest('one',2,3.333,4,5,6,7,8,9,10,11,12);
$q->$f('one',2,3.333,4,5,6,7,8,9,10);
$q->$f('one',2,3.333,4,5,6,7,8,9,10,11,12);
refTest($q);
$f = 'varArgsTest';
$q->varArgsTest('one',2,3.333,4,5,6,7,8,9,10);
$q->varArgsTest('one',2,3.333,4,5,6,7,8,9,10,11,12);
$q->$f('one',2,3.333,4,5,6,7,8,9,10);
$q->$f('one',2,3.333,4,5,6,7,8,9,10,11,12);
$f = 'varArgsTest2';
$q->varArgsTest2('one',2,3.333,4,5,6,7,8,9,10);
$q->varArgsTest2('one',2,3.333,4,5,6,7,8,9,10,11,12);
$q->$f('one',2,3.333,4,5,6,7,8,9,10);
$q->$f('one',2,3.333,4,5,6,7,8,9,10,11,12);
}
