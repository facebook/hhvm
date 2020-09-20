<?hh

// Namespace is important: we need to check that there's no user-supplied
// prefix, not just that there is no prefix at all, and AUTO332 expands to
// Foo\AUTO322

namespace Foo;

function main(): void {
  switch(\random_int(123, 456)) {
    case 123:AUTO332 // this should not invoke autocomplete
  }
}
