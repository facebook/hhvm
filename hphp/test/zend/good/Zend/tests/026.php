<?hh

class foo {
    public function a() {
    }
}
<<__EntryPoint>> function main(): void {
$test = new foo;

$test->a()->a;
print "ok\n";
}
