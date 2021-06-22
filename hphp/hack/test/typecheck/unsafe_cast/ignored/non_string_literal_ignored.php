<?hh

function m(): int {
  $msg = 'wrong';
  \HH_FIXME\UNSAFE_CAST<bool,int>(true,$msg);
}
