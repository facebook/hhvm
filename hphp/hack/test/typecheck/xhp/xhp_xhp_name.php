<?hh // strict

class :xhpchildren {
  children (xhp)*;
}

class :foochildren {
  children (foo)*;
}

class :foobarchildren {
  children (foo:bar)*;
}

class :foobarchildrenagain {
  children (:foo:bar)*;
}
