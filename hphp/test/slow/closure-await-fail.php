<?hh

function foo($x) {
  return () ==> await $x;
}

<<__EntryPoint>> function main(): void {}
