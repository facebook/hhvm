<?hh // strict

namespace T1 {
  interface I {}
}

namespace T2 {
  class A implements \T1\I {}
}
