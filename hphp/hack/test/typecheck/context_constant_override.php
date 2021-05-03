<?hh

abstract class B {
  abstract const ctx C = [defaults];
}
interface I {
  const ctx C = [read_globals];
}
class A extends B implements I {}
