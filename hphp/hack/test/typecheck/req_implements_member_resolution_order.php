<?hh // strict

class A {}
class B {}

interface IA { abstract const type T as A; }
interface IB { abstract const type T as B; }

trait X {
  require implements IB;
  require implements IA;
}

trait Y implements IA {
  use X;
}

// In Typing_extends, we want to verify that Y is compatible with X. To do so,
// we check that Y::T is a valid override of X::T. This requires us to answer
// the question: what type constraint is assigned to X::T?

// Because requirements appear in the MRO in the reverse of the order in which
// they appear syntactically, and IA is required after IB, X inherits IA::T,
// which has constraint A.

// A bug in linearization caused X to inherit IB::T instead. We incorrectly
// treated require-implements ancestors as synthesized, which causes them to be
// placed at the end of the MRO. Since we also include require-implements
// ancestors as direct, non-synthesized parents (within traits, via
// Decl_linearize.from_parent), *without reversing their order*, IB occurred
// before IA in the linearization. IB::T has constraint B, so X::T would get
// constraint B.

// Because Y implements IA (and interface constants occur before traits in the
// linearization), Y would inherit IA::T (with constraint A), which is not
// compatible with X::T (with constraint B), and we would emit an error.

// The exact pattern in this file is not usable in practice (no type can satisfy
// both IA and IB, so no class can use Y), but the same issue occurs when A and
// B are both interfaces.
