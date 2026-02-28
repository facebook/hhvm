<?hh
function test($s) :mixed{
    $v = chunk_split(base64_encode($s));
    $r = base64_decode($v, True);
    var_dump($v, $r);
}
<<__EntryPoint>> function main(): void {
test('PHP');
test('PH');
test('P');
}
