<?hh

<<__DynamicallyConstructible>>
class dyn_A{
}
 class B{
}

 <<__EntryPoint>>
function main_1205() {
$cls = 'dyn_A';
 $a = new $cls();
}
