<?hh // strict

namespace NS_Issue_109;

class C {}

function test(): void {
  $c = new C();
  $v = $c instanceof C::class;	// checker allows, but HHVM doesn't
} 

//test();