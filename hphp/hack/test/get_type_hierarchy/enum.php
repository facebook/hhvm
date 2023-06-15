<?hh
enum E1: int {}

enum E2: E1 {}
//   ^ type-hierarchy-at-caret
