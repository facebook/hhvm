<?hh

echo "*** Testing compact() : basic functionality ***\n";

$a=1;
$b=0.2;
$c=true;
$d=darray["key"=>"val"];
$e=NULL;
$f="string";

// simple array test
var_dump(compact(varray["a", "b", "c", "d", "e", "f"]));
// simple parameter test
var_dump(compact("a", "b", "c", "d", "e", "f"));
var_dump(compact(darray["keyval"=>"a", "b"=>"b", "c"=>1]));

// cases which should not yield any output.
var_dump(compact(varray[10, 0.3, true, varray[20], NULL]));
var_dump(compact(10, 0.3, true, varray[20], NULL));
var_dump(compact(varray["g"]));

echo "Done";
