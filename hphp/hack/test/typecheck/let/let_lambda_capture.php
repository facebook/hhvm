<?hh // experimental

function foo(): void {
  let count = 10;
  let repeat = () ==> {
    for ($i = 0; $i < count; $i++)
      echo $i;
  };
  repeat();
}
