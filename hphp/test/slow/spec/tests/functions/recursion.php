<?hh

function factorial($int)
:mixed{
    return ($int > 1) ? $int * factorial($int - 1) : $int;
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  // use recursion to implement a factorial function
  // Note: can call a function prior to its definition

  for ($i = 0; $i <= 10; ++$i)
  {
      printf("%2d! = %d\n", $i, factorial($i));
  }
}
