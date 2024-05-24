<?hh

namespace something;

// namespaces with '-'s shouldn't be allowed
use namespace :bar-baz;

function test(): void {}
