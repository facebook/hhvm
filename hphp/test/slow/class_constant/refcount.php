<?hh

function t() {
  if (mt_rand()) {
    include 'refcount-1.inc';
  } else {
    include 'refcount-2.inc';
  }
}


<<__EntryPoint>>
function main_refcount() {
t();
var_dump(Foo::VALUE);
}
