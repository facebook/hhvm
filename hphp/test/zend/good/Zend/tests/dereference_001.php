<?hh

function a() {
    return varray[1,varray[5]];
}

function b() {
    return varray[];
}

class foo {
    public $y = 1;

    public function test() {
        return varray[varray[varray['foobar']]];
    }
}

function c() {
    return varray[new foo];
}

function d() {
    $obj = new foo;
    return $obj->test();
}

function e() {
    $y = 'bar';
    $x = darray['a' => 'foo', 'b' => $y];
    return $x;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
var_dump(a()[1][0]); // int(5)
try { var_dump(b()[0]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
var_dump(c()[0]->y); // int(1)
var_dump(d()[0][0][0][3]); // string(1) "b"
var_dump(e()['b']); // string(3) "bar"
}
