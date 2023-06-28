<?hh

function a() :mixed{
    return varray[1,varray[5]];
}

function b() :mixed{
    return varray[];
}

class foo {
    public $y = 1;

    public function test() :mixed{
        return varray[varray[varray['foobar']]];
    }
}

function c() :mixed{
    return varray[new foo];
}

function d() :mixed{
    $obj = new foo;
    return $obj->test();
}

function e() :mixed{
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
