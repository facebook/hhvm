<?hh // strict
// KO: can't refer to a PU type with specifying a member
class PU5 {
  enum Y {
     case type S;
  }
  enum X {
     case type T;
     :@A (type T = PU5:@Y:@S);
  }
}
