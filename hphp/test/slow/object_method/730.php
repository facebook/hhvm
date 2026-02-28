<?hh

class c {
function foo() :mixed{
 echo "called
";
 }
}
function meh() :mixed{
}
function z() :mixed{
  $p = new c;
  $p->foo(meh());
  $p = null;
}

<<__EntryPoint>>
function main_730() :mixed{
z();
}
