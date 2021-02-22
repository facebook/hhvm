<?hh

abstract class B {
  abstract const ctx C = [defaults];
}
interface I {
  const ctx C = [rx];
}
class A extends B implements I {}
