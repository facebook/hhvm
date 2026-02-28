<?hh

class Klass {
  public int $value = 3;
}

<<__EntryPoint>>
function main(): void {
   $y = 1;
   $x = $y + 3;
   $y = 2;
   $x = 5; // OK to inline this definition
   /*range-start*/$x/*range-end*/;
}
