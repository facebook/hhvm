<?hh

module facebook.test;

// Global constants
const int GLOBAL_CONST = 42;
const string GLOBAL_STRING = "hello";

/**
 * This is a doc comment for the entry point
 * spanning multiple lines
 */
<<__EntryPoint>>
function main(): void {
  echo "hello";
}

// Comment before abstract class
abstract class AbstractBase<T> {
  // Hidden: private members
  private int $secret = 0;
  private function privateHelper(): void {}
  private static function privateStatic(): void {}

  // Shown: public/protected members
  public string $name = "";
  protected int $count = 0;
  public static int $instances = 0;
  protected static ?T $cached = null;

  // Constants
  const int MAX_SIZE = 100;
  abstract const int MIN_SIZE;

  // Type constants
  const type TData = shape('x' => int);
  abstract const type TItem;
  const type TDefault = string;

  // Abstract methods
  abstract public function process(T $item): void;

  // Regular methods with various modifiers
  public function greet(string $who): string {
    return "Hello, " . $who;
  }

  // Method with multiline parameters
  public function handleRequest(
    string $endpoint,
    dict<string, mixed> $params,  // inline comment
    ?ResponseFormatter $formatter,
    int $timeout_ms = 5000,
  ): Awaitable<ResponseObject> {
    // long method body
    return new ResponseObject();
  }

  // Async method
  protected static async function helper(): Awaitable<void> {}

  <<__Memoize>>
  public function memoized(int $x): int {
    return $x * 2;
  }
}

// Comment between classes
// Another comment

/**
 * Concrete implementation with doc comment
 */
final class Concrete extends AbstractBase<int>
  implements IFoo, IBar {
  const type TItem = int;
  const int MIN_SIZE = 1;

  // Static property
  private static int $counter = 0;

  <<__Override>>
  public function process(int $item): void {}

  <<__Override, __Memoize>>
  public function doSomething(int $x): void {
    // Implementation
  }

  // Constructor with multiline params
  public function __construct(
    private string $id,
    private ?Logger $logger = null,
  ) {
    parent::__construct();
  }

  <<__Memoize>>
  public static function create(): Concrete {
    return new Concrete("default");
  }

  // Method with complex return type
  public async function fetchData(
    vec<string> $ids,
  ): Awaitable<dict<string, shape('data' => mixed, 'error' => ?string)>> {
    return dict[];
  }
}

interface IFoo {
  public function doSomething(int $x): void;

  // Interface with default implementation
  public function withDefault(): string {
    return "default";
  }
}

interface IBar {
  require extends AbstractBase;

  // Interface constant
  const string VERSION = "1.0";
}

// Comment before trait
trait MyTrait {
  require extends AbstractBase;
  use OtherTrait;

  // Trait property
  private int $traitProp = 0;

  public function traitMethod(): void {}

  // Trait with complex method
  protected async function traitAsync(
    vec<Item> $items,
  ): Awaitable<vec<ProcessedItem>> {
    return vec[];
  }
}

trait OtherTrait {
  public function otherMethod(): void {}
}

// Regular enum
enum Color: string as string {
  RED = "red";
  GREEN = "green";
  BLUE = "blue";
}

// Enum class with comments
enum class Priority: int {
  int LOW = 0;  // inline comment
  int MEDIUM = 1;
  int HIGH = 2;
}

// Another enum class with more complex structure
enum class MyEnumClass: mixed {
  int FOO = 1;
  string BAR = "bar";
  shape('x' => int, 'y' => string) BAZ = shape('x' => 1, 'y' => 'baz');
}

// Type aliases
type MyAlias = shape('x' => int, 'y' => string);
type ComplexAlias = shape(
  'id' => string,
  'data' => dict<string, mixed>,
  ?'optional' => vec<int>,
  ...
);

// Newtype
newtype OpaqueId = int;
newtype OpaqueId2 as int = int;

// Case type
case type MaybeInt = int | null;
case type Result<T> = T | Error;

// Standalone functions with various signatures
function simple(): void {}

/**
 * Function with doc comment
 */
function with_doc(int $x): int {
  return $x * 2;
}

// Async function with multiline params
async function process_batch(
  vec<Item> $items,
  (function(Item): bool) $filter,  // comment
  (function(Item): ProcessedItem) $transform,
  ?CancelToken $cancel = null,
): Awaitable<vec<ProcessedItem>> {
  return Vec\map($items, $transform);
}

// Generic function
function generic<T as arraykey>(
  dict<T, mixed> $data,
): vec<T> {
  return Vec\keys($data);
}

// Function with attributes
<<__Memoize, __ConsistentConstruct>>
function with_attributes(): void {}

// Class with namespace imports at top
use namespace HH\Lib\{Vec, Dict, Str};

class UseExample {
  public function example(): void {
    $x = Vec\map(vec[1, 2, 3], $n ==> $n * 2);
  }
}

// Final class with no parent
final class StandaloneClass /* comment */ {
  // Comments everywhere
  public function method(): void {} // trailing comment
}

// Edge case: empty class
class EmptyClass {}

// Edge case: class with only private members (should be mostly hidden)
class OnlyPrivate {
  private int $x = 0;
  private function f(): void {}
  private static function s(): void {}
}

// A record (if supported in the version)
// record Point(int $x, int $y) {}

// Abstract final class (static class)
abstract final class StaticUtility {
  public static function helper(): void {}
  private static dict<string, mixed> $cache = dict[];
}

