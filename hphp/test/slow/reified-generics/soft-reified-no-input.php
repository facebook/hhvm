<?hh

class C<<<__Soft>> reify T> {}

class D extends C {}

function f<<<__Soft>> reify T>() :mixed{}

<<__EntryPoint>>
function main(): void {
	echo "functions\n";
	f();      // warn
	f<_>();   // warn
	f<int>(); // success

	echo "\nclasses\n";

	new C();      // warn
	new C<_>();   // warn
	new C<int>(); // success

	echo "\nextends classes\n";

	new D(); // warn

	echo "\ndone\n";
}
