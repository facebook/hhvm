<?hh
function foo(): void {
  $x = vec[];
  while (true) {
    $x[] = str_repeat("asd", 1000);
  }
}

<<__EntryPoint>>
function main_oom_psp(): void {
  ini_set('memory_limit', '18M');
  register_postsend_function(() ==> {
    echo "hi\n";
    foo();
  });
  foo();
}
