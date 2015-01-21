<?hh // strict

abstract class C1 {
  abstract const int X;
}
class C2 extends C1 {
  const string X = 'a'; // error: unexpected type
}
