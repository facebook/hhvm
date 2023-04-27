<?hh

class HH_FOO {}
//    ^ hover-at-caret

// This should show HH_FOO, not _FOO. This isn't an occurrence of the \HH\ namespace.
