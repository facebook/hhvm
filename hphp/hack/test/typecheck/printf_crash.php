//// def.php
<?hh // strict

newtype FormatString<T> = string;

//// printf.php
<?hh // decl
interface MyPlainSprintf {
}

function my_sprintf(FormatString<PlainSprintf> $f, ...): string;

//// use.php
<?hh // strict

final class Foo {
  public function f(FormatString $s): void {
    my_sprintf($s);
  }
}
