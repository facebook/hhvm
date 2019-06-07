<?hh

class foo {
    public function a() {
    }
}
<<__EntryPoint>> function main() {
$test = new foo;

$test->a()->a;
print "ok\n";
}
