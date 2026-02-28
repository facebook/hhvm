<?hh
function test($str) :mixed{
    $res = sha1($str)."\n";
    return $res;
}
<<__EntryPoint>> function main(): void {
echo test("");
echo test("a");
echo test("abc");
echo test("message digest");
echo test("abcdefghijklmnopqrstuvwxyz");
echo test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
echo test("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
}
