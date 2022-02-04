//// def.php
<?hh// strict

//// printf.php
<?hh
interface MyPlainSprintf {
}

function my_sprintf(\HH\FormatString<PlainSprintf> $f, mixed ...$_): string {}

//// use.php
<?hh// strict

final class Foo {
  public function f(\HH\FormatString $s): void {
    my_sprintf($s);
  }
}
