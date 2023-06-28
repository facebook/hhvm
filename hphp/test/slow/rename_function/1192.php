<?hh

function test1() :mixed{
 echo "test1
";
 }
function test3() :mixed{
 echo "test3
";
 }
function baz($test1, $test2) :mixed{
  var_dump(function_exists("Test1"));
  var_dump(function_exists("tEst2"));
  var_dump(function_exists($test1));
  var_dump(function_exists($test2));
}

<<__EntryPoint>>
function main_1192() :mixed{
baz("teSt1", "test2");
fb_rename_function("test1", "test2");
baz("TEst1", "test2");
fb_rename_function("test3", "test1");
baz("test1", "test2");
test1();
test2();
}
