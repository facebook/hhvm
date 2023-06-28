<?hh

trait T {
  require extends A;
  require implements A;
}
class X {
  use T;
}

<<__EntryPoint>>
function main_duplicate_require() :mixed{
;
}
