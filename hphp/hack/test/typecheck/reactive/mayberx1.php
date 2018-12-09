<?hh // strict

// ERROR, __AtMostRxAsFunc can only appear on parameters of
// conditionally reactive functions
<<__AtMostRxAsFunc>>
class A {
}
