<?hh // strict

namespace NS_new;

class C {}

function main(): void {
	$c1 = new C();
//	$c1 = new 'C'();
//	$name = 'C';
//	$c1 = new $name();
}

/* HH_FIXME[1002] call to main in strict*/
main();

