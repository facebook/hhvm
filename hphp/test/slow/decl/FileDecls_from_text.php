<?hh

abstract class MyOtherClass {
  private async function genNothing(): Awaitable<void> {
    $a = 1;
  }
}

<<__EntryPoint>>
function main(): void {
  // Get the autoloader to initialize
  HH\autoload_is_native();

  // === FileDecls should fail parsing this text
  $instance = HH\FileDecls::parseText('invalid code');
  echo "FileDecls Instance (should be yes):".($instance !== null ? "Yes\n" : "No\n");
  echo "FileDecls Error (should have error):".$instance->getError()."\n";

  // === FileDecls should succeed parsing this text
  $instance = HH\FileDecls::parseText('<?hh

  final class MyAttribute implements
    HH\ClassAttribute,
    HH\MethodAttribute,
    HH\TypeAliasAttribute,
    HH\EnumAttribute,
    HH\FunctionAttribute,
    HH\EnumClassAttribute,
    HH\ModuleAttribute,
    IndexableClassAttributeN<string> {

    public function __construct(string $first, string ...$remainder)[] {
    }
  }

  <<MyAttribute("a","b")>>
  class MyClass {
    const type MyTypeAlias = dict<string, MyClass>;
    const type MySimpleTypeAlias = ?int;

    const string MY_CONSTANT = "abc";
    private static int $myStaticField = 123;
    private bool $myInstanceField = true;

    <<__Deprecated("Some Message", 10000)>>
    protected final static function MyMethod(inout MySimpleTypeAlias $arg): void {}

    public async function genOtherMethod<T>()[]: Awaitable<T> { return false; }
  }
  ');

  echo "FileDecls Error (should be null):".($instance->getError()??'null')."\n";
  echo "Has class MyClass (should be yes):".($instance->hasType('MyClass') ? "Yes\n" : "No\n");
  echo "Has class NoClass (should be no):".($instance->hasType('NoClass') ? "Yes\n" : "No\n");

  echo "File: ".var_export($instance->getFile(), true)."\n";

  echo "\nSome constants:\n";
  var_dump(vec[
    HH\VISIBILITY_PROTECTED,
    HH\VARIANCE_COVARIANT,
    HH\REIFIED_ERASED,
    HH\TYPE_CONSTRAINT_KIND_AS,
    HH\CLASS_KIND_CLASS
  ]);
  echo "\n";

  $instance = HH\FileDecls::parseText("<?hh
  type InlineSeeMoreToFBECheckerInput = shape(
    'ad_impression' => ?AdImpression,
  );
  ");
  echo "FileDecls Error (should be null):".($instance->getError()??'null')."\n";
  echo "Has typedef InlineSeeMoreToFBECheckerInput (should be yes):".($instance->hasType('InlineSeeMoreToFBECheckerInput') ? "Yes\n" : "No\n");
}
