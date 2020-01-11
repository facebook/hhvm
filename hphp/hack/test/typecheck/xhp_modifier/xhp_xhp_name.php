<?hh // strict

xhp class xhp {}

xhp class nochildren {
  children empty;
}

xhp class xhpchildren {
  children (xhp)*;
}

xhp class foochildren {
  children (foo)*;
}

xhp class foobarchildren {
  children (foo:bar)*;
}
