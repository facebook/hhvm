<?hh

interface I1 {
  require extends NonExistent;
}

class X implements I1 {
}

