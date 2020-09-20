<?hh

interface if_a {
    function f_a();
}

interface if_b {
    function f_b();
}

interface if_c extends if_a, if_b {
    function f_c();
}

interface if_d extends if_a, if_b {
    function f_d();
}

interface if_e {
    function f_d();
}

interface if_f extends /*if_e,*/ if_a, if_b, if_c, if_d /*, if_e*/ {
}

class base {
    function test($class) {
        echo "is_a(" . get_class($this) . ", $class) ". (is_a($this, $class) ? "yes\n" : "no\n");
    }
}

class class_a extends base implements if_a {
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}


class class_b extends base implements if_a, if_b {
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}

class class_c extends base implements if_c {
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}

class class_d extends base implements if_d{
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}

class class_e extends base implements if_a, if_b, if_c, if_d {
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}

class class_f extends base implements if_e {
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}

class class_g extends base implements if_f {
    function f_a() {}
    function f_b() {}
    function f_c() {}
    function f_d() {}
    function f_e() {}
}

<<__EntryPoint>> function main(): void {
echo "class_a\n";

$t = new class_a();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "class_b\n";

$t = new class_b();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "class_c\n";

$t = new class_c();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "class_d\n";

$t = new class_d();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "class_e\n";

$t = new class_e();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "class_f\n";

$t = new class_f();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "class_g\n";

$t = new class_g();
echo $t->test('if_a');
echo $t->test('if_b');
echo $t->test('if_c');
echo $t->test('if_d');
echo $t->test('if_e');

echo "===DONE===\n";
}
