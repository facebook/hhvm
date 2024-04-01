<?hh

namespace {
    use function foo\bar;
    use function foo\BAR; // ok because functions are case sensitive
    use function foo\bar; // error, duplicate
}
