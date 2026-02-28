<?hh


<<__EntryPoint>>
function main_isset_elem() :mixed{
$a = vec[0, 1, 2];
print ":".(string)(isset($a[-1])).":\n";
print ":".(string)(isset($a[0])).":\n";
print ":".(string)(isset($a[1])).":\n";
print ":".(string)(isset($a[3])).":\n";
print ":".(string)(isset($a["blah"])).":\n";
print "\n";

$a = vec[null, null];
print ":".(string)(isset($a[-1])).":\n";
print ":".(string)(isset($a[0])).":\n";
print ":".(string)(isset($a[1])).":\n";
print ":".(string)(isset($a[2])).":\n";
print "\n";

$a = "";
print ":".(string)(isset($a[-1])).":\n";
print ":".(string)(isset($a[0])).":\n";
print ":".(string)(isset($a[1])).":\n";
print ":".(string)(isset($a["blah"])).":\n";
print ":".(string)(isset($a[0][0])).":\n";
print "\n";

$a = "a01";
print ":".(string)(isset($a[-1])).":\n";
print ":".(string)(isset($a[0])).":\n";
print ":".(string)(isset($a[1])).":\n";
print ":".(string)(isset($a[2])).":\n";
print ":".(string)(isset($a[3])).":\n";
print ":".(string)(isset($a["blah"])).":\n";
print ":".(string)(isset($a[0][0])).":\n";
print "\n";

$a = 0;
print ":".(string)(isset($a[0])).":\n";
print "\n";

$a = 42;
print ":".(string)(isset($a[0])).":\n";
print "\n";

$a = false;
print ":".(string)(isset($a[0])).":\n";
print "\n";

$a = 2.0;
print ":".(string)(isset($a[0])).":\n";
print "\n";
}
