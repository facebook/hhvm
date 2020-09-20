<?hh

class c {
function foo() {
 echo "called
";
 }
}
function meh() {
}
function z() {
  $p = new c;
  $p->foo(meh());
  $p = null;
}

<<__EntryPoint>>
function main_730() {
z();
}
