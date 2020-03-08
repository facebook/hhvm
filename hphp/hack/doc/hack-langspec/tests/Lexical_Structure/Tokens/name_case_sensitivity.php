<?hh // strict

namespace NS_name_case_sensitivity;

function F(): void {}
//function f(): void {}	// name spelling differs only in capitalization

class C {
	public function F(): void {}
	public function f(): void {}
}
//class c {}		// not permitted

interface I {}
//interface i {}	// not permitted

trait T {}
//trait t {}		// not permitted

enum E: int {}
//enum e: int {}	// not permitted

type T1 = int;
//type t1 = float;	// not permitted

newtype NT1 = int;
//newtype NT1 = float;	// not permitted

function main(): void {
}

/* HH_FIXME[1002] call to main in strict*/
main();
