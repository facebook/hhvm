<?hh

function postInc(inout $x) :mixed{
  return $x++;
}

function preInc(inout $x) :mixed{
  return ++$x;
}

function postDec(inout $x) :mixed{
  return $x--;
}

function preDec(inout $x) :mixed{
  return --$x;
}
<<__EntryPoint>> function main(): void {
$x = 2;
var_dump(postInc(inout $x));
var_dump($x);
var_dump(preInc(inout $x));
var_dump($x);
var_dump(postDec(inout $x));
var_dump($x);
var_dump(preDec(inout $x));
var_dump($x);

$y = 2.5;
var_dump(postInc(inout $y));
var_dump($y);
var_dump(preInc(inout $y));
var_dump($y);
var_dump(postDec(inout $y));
var_dump($y);
var_dump(preDec(inout $y));
var_dump($y);

$f = false;
var_dump(postInc(inout $f));
var_dump($f);
var_dump(preInc(inout $f));
var_dump($f);
var_dump(postDec(inout $f));
var_dump($f);
var_dump(preDec(inout $f));
var_dump($f);

$t = true;
var_dump(postInc(inout $t));
var_dump($t);
var_dump(preInc(inout $t));
var_dump($t);
var_dump(postDec(inout $t));
var_dump($t);
var_dump(preDec(inout $t));
var_dump($t);
}
