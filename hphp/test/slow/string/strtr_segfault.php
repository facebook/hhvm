<?hh

// at least 64 patterns required to trigger the WuManberReplacement algorithm
<<__EntryPoint>>
function main_strtr_segfault() :mixed{
$map = dict[];
for ( $i = 0; $i < 66; ++$i ) {
  $map['aaa'.$i] = 'x';
}
// source string must have fewer bytes then the smallest pattern above
echo strtr( 'ab', $map ), "\n";
}
