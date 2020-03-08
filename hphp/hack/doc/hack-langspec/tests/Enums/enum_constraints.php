<?hh // strict

namespace NS_enum_constraints;

enum E2: int { Z = 12; }
enum E3: string { X = 'aa'; }

//enum E: int as bool {		// types are incompatible
//enum E: int as int {		// Okay
//enum E: int as float {	// types are incompatible
//enum E: int as num {		// Okay
//enum E: int as arraykey {	// Okay
//enum E: int as ?int {		// Okay
//enum E: int as string {	// types are incompatible
//enum E: int as void {		// types are incompatible
//enum E: int as string {	// types are incompatible
//enum E: int as E2 {		// types are incompatible
//enum E: int as E3 {		// types are incompatible
enum E: int as mixed {		// Okay
  A = 1;
  B = 2;
}

enum F2: int { Z = 12; }
enum F3: string { X = 'aa'; }

//enum F: string as bool {	// types are incompatible
//enum F: string as int {	// types are incompatible
//enum F: string as float {	// types are incompatible
//enum F: string as num	{	// types are incompatible
//enum F: string as arraykey {	// Okay
//enum F: string as string {	// Okay
//enum F: string as ?string {	// Okay
//enum F: string as void {	// types are incompatible
//enum F: string as F2 {	// types are incompatible
//enum F: string as F3 {	// types are incompatible
enum F: string as mixed {	// Okay
  A = 'zz';
  B = 'xx';
}

function main(): void {
}

/* HH_FIXME[1002] call to main in strict*/
main();

