<?hh

// fine, provided that all members of C are SDT

<<__SupportDynamicType>>
trait T {
  require extends C;
}

class C {
}

<<__SupportDynamicType>>
class D extends C {
  use T;
}
