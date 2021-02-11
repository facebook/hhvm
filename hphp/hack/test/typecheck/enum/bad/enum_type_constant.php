<?hh //strict

enum E:int as int {}

// Should fail since E is a newtype for int, not a class
function f(): E::class {}
