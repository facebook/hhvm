<?hh
<<__EntryPoint>> function main(): void {
include(dirname(__FILE__) . '/../../../../tests/quicktester.inc');

quicktester(() ==> 'ABC', () ==>  trim('ABC'));
quicktester(() ==> 'ABC', () ==> ltrim('ABC'));
quicktester(() ==> 'ABC', () ==> rtrim('ABC'));
quicktester(() ==> 'ABC', () ==>  trim(" \0\t\nABC \0\t\n"));
quicktester(() ==> "ABC \0\t\n", () ==> ltrim(" \0\t\nABC \0\t\n"));
quicktester(() ==> " \0\t\nABC", () ==> rtrim(" \0\t\nABC \0\t\n"));
quicktester(() ==> " \0\t\nABC \0\t\n", () ==>  trim(" \0\t\nABC \0\t\n",''));
quicktester(() ==> " \0\t\nABC \0\t\n", () ==> ltrim(" \0\t\nABC \0\t\n",''));
quicktester(() ==> " \0\t\nABC \0\t\n", () ==> rtrim(" \0\t\nABC \0\t\n",''));
quicktester(() ==> "ABC\x50\xC1", () ==> trim("ABC\x50\xC1\x60\x90","\x51..\xC0"));
quicktester(() ==> "ABC\x50", () ==> trim("ABC\x50\xC1\x60\x90","\x51..\xC1"));
quicktester(() ==> "ABC", () ==> trim("ABC\x50\xC1\x60\x90","\x50..\xC1"));
quicktester(() ==> "ABC\x50\xC1", () ==> trim("ABC\x50\xC1\x60\x90","\x51..\xC0"));
quicktester(() ==> "ABC\x50", () ==> trim("ABC\x50\xC1\x60\x90","\x51..\xC1"));
quicktester(() ==> "ABC", () ==> trim("ABC\x50\xC1\x60\x90","\x50..\xC1"));
}
