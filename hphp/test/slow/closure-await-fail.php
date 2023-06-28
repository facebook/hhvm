<?hh

function foo($x) :mixed{
  return () ==> await $x;
}

