<?hh

// fine, as long as all members of T are SDT (following the usual SDT
// trait use rules)

<<file: __EnableUnstableFeatures('require_class') >>

trait T {
  require class C;
}

<<__SupportDynamicType>>
final class C {
  use T;
}
