<?hh

function f() :mixed{
  echo "done\n";
}

<<__EntryPoint>>
function main(): void {
  f(1, 2);
  f(1, 2, ...vec[]);
  f(1, ...vec[2]);
  f(...vec[1, 2]);

  f(1);
  f(1, ...vec[]);
  f(...vec[1]);

  f();
  f(...vec[]);
}
