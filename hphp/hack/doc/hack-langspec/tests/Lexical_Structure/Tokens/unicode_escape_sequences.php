<?hh // strict

namespace NS_unicode_escape_sequences;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

/* As of 2016-02-06, unicode escape sequences longer than 6 chars are rejected, even if their values are
in-range, but just have extra leading zeros. This seems like a bug. */ 

function main(): void {
  echo "================= Basic checks =================\n\n";

  var_dump(">\u{41}<");		// results in ">A<", length 3
  var_dump(">\U{41}<");		// results in ">\U{41}<", length 8
  var_dump(">\u{041}<");		// results in ">A<", length 3
  var_dump(">\u{0041}<");		// results in ">A<", length 3
  var_dump(">\u{00041}<");		// results in ">A<", length 3
  var_dump(">\u{000041}<");		// results in ">A<", length 3
//  var_dump(">\u{0000041}<");		// results in ">A<", length 3
//  var_dump(">\u{00000041}<");		// results in ">A<", length 3
//  var_dump(">\u{000000000000000000000041}<");		// results in ">A<", length 3

  var_dump(">\u{A9}<");		// results in ">©<", length 4
  var_dump(">\u{a9}<");		// results in ">©<", length 4
  var_dump(">\u{0000A9}<");		// results in ">©<", length 4
//  var_dump(">\u{00000000A9}<");		// results in ">©<", length 4

  var_dump(">\u{e6}<");		// results in ">æ<", length 4

  var_dump(">\u{2020}<");		// results in ">†<", length 5

  var_dump(">\u{20000}<");		// results in ">𠀀<", length 6
  var_dump(">\u{020000}<");		// results in ">𠀀<", length 6
//  var_dump(">\u{0020000}<");		// results in ">𠀀<", length 6

  var_dump(">\u{10FFFF}<");		// results in ">𠀀<", length 6

//  var_dump(">\u{110000}<");		// Invalid UTF-8 codepoint escape sequence: Codepoint too large (> 0x10FFFF)

  echo "\n================= Is variable substitution allowed? =================\n\n";

  $v = "41";
  var_dump(">$v<");				// string(4) ">41<"
  var_dump(">0$v<");				// string(5) ">041<"
  var_dump(">0{$v}<");			// string(5) ">041<"

  var_dump(">\u{$v}<");			// string(6) ">\u41<"
//  var_dump(">\u{0$v}<");		// Invalid UTF-8 codepoint escape sequence
//  var_dump(">\u{{$v}}<");		// Invalid UTF-8 codepoint escape sequence
}

/* HH_FIXME[1002] call to main in strict*/
main();
