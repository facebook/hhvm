<?hh // strict

class foo {}

class :foo:bar {}

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
