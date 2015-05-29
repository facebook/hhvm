<?hh

// This should be a parse error (it is in HHVM), but we previously *crashed*.
function namespace() {
  return 'why';
}
