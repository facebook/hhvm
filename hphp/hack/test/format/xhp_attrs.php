<?hh

class :foo {
  attribute int a;
  attribute int aa, string bb;
  attribute int xxxxx, :foo yyyyy, string zzzzz,
    :foo qqqqq, :foo rrrrr, :foo sssss;

  attribute
    /* some comment -- preserve the newline after me! */
    int foo,

    /* some comment */
    string bar,

    Stringish baz, // blah blah

    Stringish qux; // blahhhh

  attribute
    enum {'aaaaaaaaaa', 'bbbbbbbbbb', 'cccccccccc', 'dddddddddd'} x = 'aaaaaaaaaa';

  children (:x:yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy|:a:bbbbbbbbbb);
}
