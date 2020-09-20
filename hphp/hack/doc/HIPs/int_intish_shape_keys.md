# Feature Name: int/intish shape keys
## Start Date: 2020-06-29
## Status: Draft

# Summary:

Shape keys are currently required to be:
- string literals that do not start with an int
- class constants

This proposal is to permit any `arraykey` literal, in addition to class
constants. More specifically:
- permit int literal shape keys
- permit any string literal shape key, even if they would be converted to ints
  by PHP

# Feature motivation:

- The ban on int-like shape keys is a PHPIsm in disguise: this exists because
  of array-key-cast behavior which no longer exists in hack.
  - The ban on actual int shape keys was to avoid any confusion with int-like
    keys, however there are other reasons (e.g. avoiding confusion with tuples)
- The bans are not actually consistently enforced, due to permitting type constants
- HSL Regex is already special and creates shapes with integer keys. It would
  be useful to be able to type regexp patterns and matches fully as they're
  passed around
- This would remove some special-casing from the typechecker

# User experience: 

Generally as already-present for `Regex\Match`; additionally, this will now be
possible:

```
// Takes any regex, returns the entire matched string
function helper<T as Regex\Pattern(shape(0 => string, ...))>(
  T $pattern,
): string {
  return Regex\Match($pattern);
}
```

Currently, it can only be typed as `Regex\Match`, which is declared as
`shape(...)` and it is impossible to refine it or redeclare it to say that
`0` (entire string) or any specific numbered captures are presnet.

# IDE experience:

Already present for Regex\Match; no change expected.

# Implementation details:

* How the feature will interact with other Hack features
  - will make HSL Regex more consistent; other interactions as already applying
* How the feature will be implemented
  - removal of restrictions/special-casing
* A description of corner cases
  - existing problems with class constants
* Any changes to HHVM
  - none
* If applicable, strategies for codemodding
  - n/a

# Design rationale and alternatives:

Largely covered by 'motivation' above. No alternatives have been considered
for removal of ban on 'int-like' `string` keys. These are a PHPism.

## Alternatives for the Regexp problem

The rest of this section addresses alternatives for actual `int` shape keys.

### Using int-like string keys positional captures

For example, `$shape['0']`. The main problem is potential future issues: while
PCRE currently bans named capture groups that start with a number, the syntax
for referncing them differs; it appears that it would be possible to remove
this restriction without breaking compatibility, *unless* we introduce this
syntax.

This would also have a minor drawback from usability/familiarity benefit, as it
would be different to the representations in all other languages.

### Using objects with getters

e.g. `->getNameCapture(string $name)`, `->getPositionalCapture(int $idx)`

This is the approach taken by most other languages/libraries that support named
captures.

This would remove the need for any changes to shapes and tuples, however to
maintain the same static safety that we currently have (i.e. we know which
named and positional captures are valid), these objects will in turn need
to be special-cased - for example, perhaps `re"/(foo(?<bar>baz))/"` is infered
to be a `RegexpPattern<tuple(string, string), shape('bar' => string)>` - however,
if tuples are used as part of the generic, changes will be needed to support
subtyping.

### Tuples

These natively support sequential integer keys, however they have several
drawbacks here:

- it's valid to mix named and positional captures (and common, especially for 0);
  in practice, would likely return  both a tuple and shape, especially from some APIs
- no sub-typing; in particular, most requests for specifying a type are for "I want
  to take any regexp with at least these captures" - for example:
  - whole string, and a specific named capture: `shape(0 => string, 'foo' => string, ...)`
    - Perhaps this would be `((string, ...), shape('foo' => string, ...)`
  - at least one position capture: `shape(1 => string, ...)`
    - perhaps `((string, string, ...), shape(...))`

Combined with the fact that all elements are the same type, this problem feels
like it would be better solved by bounded-size vecs - i.e. a `vec<string>` with
at least `n` elements - however, in the regexp case, the user normally cares
about presence of a specific n, not `0..=n`, which is a problem already addressed
by shapes.

# Drawbacks: 

- may be misused when tuples are a better fit
- can not be converted to be class-based in the future
  - unlikely due to existing COW semantics
  - likely already impractical due to class constant hole and lack of runtime enforcement
- can not be converted to 'data classes' (a.k.a. 'records') in the future
  - likely already impractical due to class constant hole and lack of runtime enforcement

# Unresolved questions: 

# Future possibilities:

- remove class constant support

