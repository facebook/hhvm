<?hh

namespace something;

// namespaces alias with ':'s shouldn't be allowed
use namespace somethingelse as :foo:bar-baz;



function test(): void {}
