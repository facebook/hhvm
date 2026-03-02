<?hh

class :x:y {
  public function f(): mixed {
    return <z> 
      {
        !true ? W`${null}` : null
      }
    </z>;
  }
}

<<__EntryPoint>>
function main(): void {
  echo "done\n";
}
