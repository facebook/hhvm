<?hh

class A<T> {}
class B<T> {}

function f<T>($x) :mixed{
  return $x is A<B<T>>;
}


/*
function f<reify Ta, reify Tb>(Ta $x): Tb {
  echo "yep\n";
  return 1;
}

f<string, int>("hi"); // pass
f<int, int>("hi"); // param fail
 */
<<__EntryPoint>>
function main_entry(): void {

  var_dump(f<int>(1));
}
