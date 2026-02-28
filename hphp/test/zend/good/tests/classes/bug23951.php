<?hh

const FOO1 = 1;
const FOO2 = 2;

class A {

    public $a_var = dict[FOO1=>'foo1_value', FOO2=>'foo2_value'];

}

class B extends A {

    public $b_var = 'foo';

}
<<__EntryPoint>> function main(): void {
$a = new A;
$b = new B;

print_r($a);
print_r($b->a_var);
print_r($b->b_var);
}
