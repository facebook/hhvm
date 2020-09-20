<?hh

function myfunc($a, $b) {
  error_log($a.$b);
}

class MyClass {
  function myMeth($a, $b) {
    error_log($a.$b);
  }
}
<<__EntryPoint>>
function entrypoint_list(): void {
  $h = new SplMaxHeap();
  $h->insert(1);
}
