<?hh

namespace NS1 // error1038, and recover to declaration level
  class C1 { }
  interface I1 { }
  trait T1 { }

namespace NS2; // no error
class C2 { }
interface I2 { }
trait T2 { }

namespace NS3 { // no error
  class C3 { }
  interface I3 { }
  trait T3 { }
}

// NS4 demonstrates a case of suboptimal error recovery. The hypothetical
// programmer writing it probably just forgot a left brace after 'NS4', but the
// FFP reports error1038 and an extra right brace. Still, this isn't too
// bad--these messages should do a reasonable job of nudging the programmer
// towards the actual mistake.
// TODO T20730184: currently the FFP gives the error "An expression is expected
// here." when encountering an unexpected right brace. Change this to something
// clearer, like "This right brace does not have a corresponding left brace."
namespace NS4 // error1038, and recover to declaration level
  class C4 {}
  interface I4 { }
  trait T4 { }
} // report extra right brace here

namespace NS5 {
  class C5 { }
  interface I5 { }
  trait T5 { } // report missing right brace here
