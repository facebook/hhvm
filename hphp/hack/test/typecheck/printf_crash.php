//// def.php
<?hh // strict

//// printf.php
<?hh // partial
interface MyPlainSprintf {
}

function my_sprintf(\HH\FormatString<PlainSprintf> $f, ...$_): string;

//// use.php
<?hh // strict

final class Foo {
  public function f(\HH\FormatString $s): void {
    my_sprintf($s);
  }
}
