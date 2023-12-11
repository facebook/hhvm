<?hh

function test($enc, $map) :mixed{
	print "$enc\n";

	foreach($map as $fromc => $toc) {
		$ustr = mb_convert_encoding(pack('C', $fromc), 'UCS-4BE', $enc);
		foreach (unpack('Nc', $ustr) as $unic);
		printf("0x%04x, 0x%04x\n", $toc, $unic);
	}
}
<<__EntryPoint>>
function main_entry(): void {
  test('ISO-8859-7',  dict[0xa4 => 0x20ac, 0xa5 => 0x20af, 0xaa => 0x037a]);
  test('ISO-8859-8',  dict[0xaf => 0x00af, 0xfd => 0x200e, 0xfe => 0x200f]);
  test('ISO-8859-10', dict[0xa4 => 0x012a                                ]);
}
