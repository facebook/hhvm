<?hh

function t() :mixed{
  if (mt_rand()) {
    include 'refcount-1.inc';
  } else {
    include 'refcount-2.inc';
  }
}


<<__EntryPoint>>
function main_refcount() :mixed{
t();
var_dump(Foo::VALUE);
}
