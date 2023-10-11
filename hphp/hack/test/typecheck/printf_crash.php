//// def.php
<?hh

//// printf.php
<?hh
interface MyPlainSprintf {
}

function my_sprintf(\HH\FormatString<PlainSprintf> $f, mixed ...$_): string {}

//// use.php
<?hh

final class Foo {
  public function f(\HH\FormatString $s): void {
    my_sprintf($s);
  }
}
