<?hh // strict

// ERROR, __MaybeRx can only appear on parameters of
// conditionally reactive functions
<<__MaybeRx>>
class A {
}
