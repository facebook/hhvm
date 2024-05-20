<?hh

namespace something;

// namespaces with ':'s shouldn't be allowed
use namespace :foo:bar-baz;

function test(): void {}
