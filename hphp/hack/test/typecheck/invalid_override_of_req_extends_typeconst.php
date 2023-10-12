<?hh // strict

class C {
  const type T = nonnull;
}

interface I {
  require extends C;
  abstract const type T as num;
  // Normally, it is an error to override a concrete typeconst with an abstract
  // one. This error is emitted by the Typing_extends module. However, we do not
  // verify required ancestors with Typing_extends, so we do not check that the
  // members of I are valid overrides of the members of C. As a result, we have
  // an invalid override here--the abstract definition of T cannot override the
  // concrete one inherited from C--but no error is emitted.
}

interface J extends I {}
// In legacy decl, no error is emitted at all, because when we inherit from I,
// we copy over its methods table into J, silently propagating the incorrect
// override down the inheritance hierarchy. However, in shallow decl, we
// consider I::T to be canonical only *within* I. In J, we conclude that the
// concrete type constant C::T is inherited instead (since concrete
// implementations take precedence over abstract specifications), and emit an
// error when Typing_extends checks that J::T is a valid override of I::T.
