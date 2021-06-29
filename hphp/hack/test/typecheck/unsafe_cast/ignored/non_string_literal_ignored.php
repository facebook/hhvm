<?hh

function m(): int {
  $msg = 'wrong';
  \HH\FIXME\UNSAFE_CAST<bool,int>(true,$msg);
}
