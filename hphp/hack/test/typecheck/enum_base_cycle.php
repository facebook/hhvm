<?hh

enum E : E {}

enum E0 : E1 {}

enum E1 : E2 {}

enum E2 : E0 {}

/* enum XXX : self {} // not allowed */
