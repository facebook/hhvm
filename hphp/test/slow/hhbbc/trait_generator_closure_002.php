<?hh

async function heh() { return 2; }

trait T {
  function f() {
    return async function() {
      return await heh();
    };
  }
}

class A {
  use T;
}

class B {
  use T;
}

async function main() {
  $a = new A;
  $f = $a->f();
  $f = await $f();
  var_dump($f);
}


<<__EntryPoint>>
function main_trait_generator_closure_002() {
main();
}
