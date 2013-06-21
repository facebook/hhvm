<?hh

// non newtypes can't have constraints
type Foo as int = int; // error
