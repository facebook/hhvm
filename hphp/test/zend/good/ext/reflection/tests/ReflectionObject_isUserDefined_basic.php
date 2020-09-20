<?hh
class C {
}
<<__EntryPoint>> function main(): void {
$r1 = new ReflectionObject(new stdClass);
$r2 = new ReflectionObject(new ReflectionClass('C'));
$r3 = new ReflectionObject(new ReflectionProperty('Exception', 'message'));
$r4 = new ReflectionObject(new Exception);
$r5 = new ReflectionObject(new C);

var_dump($r1->isUserDefined(), $r2->isUserDefined(), $r3->isUserDefined(),
         $r4->isUserDefined(), $r5->isUserDefined());
}
