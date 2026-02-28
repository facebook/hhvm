<?hh

function error_boundary($fn) :mixed{
  try {
    return $fn();
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
    return null;
  }
}

function postInc($x) :mixed{
  return error_boundary(() ==> $x++);
}

function preInc($x) :mixed{
  return error_boundary(() ==> ++$x);
}

function postDec($x) :mixed{
  return error_boundary(() ==> $x--);
}

function preDec($x) :mixed{
  return error_boundary(() ==> --$x);
}
<<__EntryPoint>> function main(): void {
var_dump(postInc(2));
var_dump(preInc(2));
var_dump(postDec(2));
var_dump(preDec(2));

var_dump(postInc(2.5));
var_dump(preInc(2.5));
var_dump(postDec(2.5));
var_dump(preDec(2.5));

var_dump(postInc(false));
var_dump(preInc(false));
var_dump(postDec(false));
var_dump(preDec(false));

var_dump(postInc(true));
var_dump(preInc(true));
var_dump(postDec(true));
var_dump(preDec(true));

var_dump(postInc(null));
var_dump(preInc(null));
var_dump(postDec(null));
var_dump(preDec(null));
}
