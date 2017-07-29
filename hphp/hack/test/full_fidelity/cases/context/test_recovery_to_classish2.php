<?hh

// This test simply exists to demonstrate a suboptimal case of error recovery
// in the FFP. If you break this test, it doesn't necessarily mean you've done
// anything wrong, since the FFP never behaved ideally on this code.

// Ideally, this code would produce an error indicating that nested classes are
// forbidden. But right now, the FFP assumes the programmer did *not* intend
// to nest classes, and throws the errors commented below.

class C1 { // missing right brace

  class C2 {
  }

} // missing expression, missing semicolon
