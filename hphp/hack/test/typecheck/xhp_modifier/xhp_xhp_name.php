<?hh // strict

xhp class xhp {}

xhp class nochildren {
  children empty;
}

xhp class foo {}

xhp class xhpchildren {
  children (xhp)*;
}

xhp class foochildren {
  children (foo)*;
}
