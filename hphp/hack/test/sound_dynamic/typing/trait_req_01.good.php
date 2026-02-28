<?hh

// This is accepted.  Any class that uses T must be SDT because it
// must implement I, and SDT classes can use non-SDT traits assuming
// all trait elements are themselves SDT.

<<__SupportDynamicType>>
interface I {}

trait T {
  require implements I;
}
