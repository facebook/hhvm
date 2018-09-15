<?hh // strict

// ERROR, __OnlyRxIfRxFunc can only appear on parameters of
// conditionally reactive functions
<<__OnlyRxIfRxFunc>>
class A {
}
