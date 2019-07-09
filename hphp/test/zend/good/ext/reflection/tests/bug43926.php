<?hh

class E {
}
class D extends E {
}

class A extends D {
}

class C extends A {
}
<<__EntryPoint>> function main(): void {
$ra = new ReflectionClass('A');
$rc = new ReflectionClass('C');
$rd = new ReflectionClass('D');
$re = new ReflectionClass('E');

$ca = $ra->newInstance();
$cc = $rc->newInstance();
$cd = $rd->newInstance();
$ce = $re->newInstance();

print("Is? A ". ($ra->isInstance($ca) ? 'true' : 'false') .", instanceof: ". (($ca is A) ? 'true' : 'false') ."\n");
print("Is? C ". ($rc->isInstance($ca) ? 'true' : 'false') .", instanceof: ". (($ca is C) ? 'true' : 'false') ."\n");
print("Is? D ". ($rd->isInstance($ca) ? 'true' : 'false') .", instanceof: ". (($ca is D) ? 'true' : 'false') ."\n");
print("Is? E ". ($re->isInstance($ca) ? 'true' : 'false') .", instanceof: ". (($ca is E) ? 'true' : 'false') ."\n");
print "-\n";
print("Is? A ". ($ra->isInstance($cc) ? 'true' : 'false') .", instanceof: ". (($cc is A) ? 'true' : 'false') ."\n");
print("Is? C ". ($rc->isInstance($cc) ? 'true' : 'false') .", instanceof: ". (($cc is C) ? 'true' : 'false') ."\n");
print("Is? D ". ($rd->isInstance($cc) ? 'true' : 'false') .", instanceof: ". (($cc is D) ? 'true' : 'false') ."\n");
print("Is? E ". ($re->isInstance($cc) ? 'true' : 'false') .", instanceof: ". (($cc is E) ? 'true' : 'false') ."\n");
print "-\n";
print("Is? A ". ($ra->isInstance($cd) ? 'true' : 'false') .", instanceof: ". (($cd is A) ? 'true' : 'false') ."\n");
print("Is? C ". ($rc->isInstance($cd) ? 'true' : 'false') .", instanceof: ". (($cd is C) ? 'true' : 'false') ."\n");
print("Is? D ". ($rd->isInstance($cd) ? 'true' : 'false') .", instanceof: ". (($cd is D) ? 'true' : 'false') ."\n");
print("Is? E ". ($re->isInstance($cd) ? 'true' : 'false') .", instanceof: ". (($cd is E) ? 'true' : 'false') ."\n");
print "-\n";
print("Is? A ". ($ra->isInstance($ce) ? 'true' : 'false') .", instanceof: ". (($ce is A) ? 'true' : 'false') ."\n");
print("Is? C ". ($rc->isInstance($ce) ? 'true' : 'false') .", instanceof: ". (($ce is C) ? 'true' : 'false') ."\n");
print("Is? D ". ($rd->isInstance($ce) ? 'true' : 'false') .", instanceof: ". (($ce is D) ? 'true' : 'false') ."\n");
print("Is? E ". ($re->isInstance($ce) ? 'true' : 'false') .", instanceof: ". (($ce is E) ? 'true' : 'false') ."\n");
}
