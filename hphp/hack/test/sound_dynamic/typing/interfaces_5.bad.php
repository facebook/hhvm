<?hh

<<__SupportDynamicType>>
interface I {}

<<__SupportDynamicType>>
trait T implements I {}

class C {
  use T;
}
