<?hh

function foo() {
  $this = 2;
  echo "You should not see this";
 }

 <<__EntryPoint>>
function main_1491() {
foo();
}
