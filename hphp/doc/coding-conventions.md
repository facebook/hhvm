HHVM Coding Conventions
=======================

This document is meant to serve as a guide to writing C++ in the HHVM codebase,
covering when and how to use various language features as well as how code
should be formatted. Our goal is to ensure a consistently high-quality codebase
that is easy to read and contribute to, especially for newcomers.

The HHVM codebase contains a wide variety of code from many different authors.
It's been through a few different major stages in its life, including stints in
multiple different repositories. As a result, large (primarily older) parts of
the codebase do not fit this guide. When in doubt about how to write or format
something, always prefer the advice here over existing conventions in the
code. If you're already touching some older code as part of your work, please
do clean it up as you go along. But please do not spend hours applying the
formatting guidelines here to code you aren't otherwise modifying. While we'd
love for the entire codebase to follow this guide, we'd rather get there
gradually than lose lots of git history and developer time to purely cosmetic
changes. That said, if cosmetic changes that you're making as part of a larger
diff keep growing in scope, it may be worth pulling them out into a separate
diff.

There's no well-defined cutoff here - just try to minimize effort for your
reviewers. A good rule of thumb is that if your cosmetic changes require adding
significant new sections to the diff (such as a function rename that touches
all callsites), it should probably be pulled out into its own diff.


## Headers ##

Every .cpp file in the HHVM repository should have a corresponding .h file with
the same name, and which declares its public interfaces. We tend to value API
documentation more heavily than inline implementation comments, so *all*
declarations in headers (classes, enums, functions, constants, etc.)  should be
documented. See Comments and Documentation for more details.

Build times are a frequent source of pain in many large C++ projects. Try not
to make large header files that mostly serve to include groups of other large
header files. This can discourage "include what you use," discussed in the
"What to include section".

### Include guards ###

To prevent multiple inclusion, all headers should have the following directive
after their license header comment:

```cpp
/*
 * ...see the 'File copyright' section for details on what goes here...
 */

#pragma once

// File contents
```

### What to include ###

The golden rule for what to include is "include what you use" (IWYU). In brief,
this means you should not rely on any headers you include to transitively
include other headers which have definitions you require. You should also
prefer to forward declare structs and classes when the definition is not needed
(so, "don't include what you don't use"), which helps reduce HHVM's nontrivial
build time.

To make it easier to achieve IWYU, we have the following guidelines for
includes:

- Always include the corresponding .h for a .cpp first, before even system
  headers.
- Separate includes into groups: C++ standard library headers, external projects
  (such as Boost and Intel TBB), and finally headers within HHVM. Each group
  should be separated by a newline, for readability. (Whether to separate HHVM
  includes by subsystem (e.g., `jit`) is left up to the author.)
- Keep headers alphabetized within each group. This makes it easier to ensure
  that all necessary includes are made, and no extraneous ones are left behind.
- Use double quotes for Folly and HHVM headers and angle brackets for all
  others.

As an example, here is what the include section might look like for a file
named `bytecode.cpp`:

```cpp
#include "hphp/runtime/vm/bytecode.h"

#include <cstdio>
#include <string>

#include <boost/program_options/options_description.hpp>

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/util/string.h"
```

### Inline functions ###

Defining functions inline is encouraged for very short functions.

When defining inline member functions on structs or classes which have tight,
compact interfaces (e.g., a smart pointer class, or any wrapper class), prefer
to define the functions in the class definition, for concision.

However, for classes with more complex, malleable APIs where inline helpers
proliferate (e.g., Func, Class, IRInstruction, etc.), restrict the class
definition to member function prototypes *only*. This makes the API much
cleaner. For these classes, define all inline functions in a corresponding
`-inl.h` file.

```cpp
// At the bottom of func.h.

#include "hphp/runtime/vm/func-inl.h"
```

```cpp
// After the copyright in func-inl.h.

namespace HPHP {

// Definitions go here.

}
```

For API's large enough to warrant -inl.h files, move *all* definitions into the
-inl.h, even one-line accessors. This serves both to keep the API cleaner and
to avoid splitting implementations among three files (the header, the inline,
and the source).

Some files, with or without a corresponding -inl.h file, may need a -defs.h
file. This file also contains definitions of inline functions, but it is *not*
included by the main header. It is intended to be used when only a few callers
need access to the definitions, or when the definitions can't be in the main
header because it would create circular dependencies. It should be included
directly by the callers that do need access to the definitions it contains.


## Structs and Classes ##

Classes are used extensively throughout the HHVM codebase, with a number of
coding conventions. See also Naming for conventions around class naming.

### Using struct vs. class ###

In C++, `struct` and `class` have nearly identical meanings; the only
difference lies in the default accessibility (`struct` defaults to public, and
`class`, to private).

We do not assign further meaning to these keywords, so we use `struct`
everywhere.  Efforts to compile under MSVC also require that we use the same
keyword between a struct/class definition and its forward declarations due to
MSVC's failure to adhere to the C++ spec, and sticking to `struct` everywhere
makes this easier.

### Access control ###

Try to avoid the `protected` keyword. It tends to give a false sense of
security about encapsulation: since anyone can inherit from your class, anyone
can access the `protected` member with a little extra effort.

### Implicit and explicit constructors ###

By default, always use `explicit` for single-argument, non-initializer list
constructors.

```cpp
struct MyStruct {
  // We don't want to implicitly convert ints to MyStructs
  explicit MyStruct(int foo);

  // Two-argument constructor; no need for explicit
  MyStruct(const std::string& name, int age);
};
```

### Public data members vs. getters/setters ###

Prefer declaring public member variables to using getters and setters. Getters
and setters that don't manage object state in a nontrivial way serve to bloat
the API and introduce unnecessary boilerplate.

Getters are, of course, encouraged for private members. Avoid prefixing getters
with `get`:

```cpp
struct Func {
  const SVInfoVec& staticVars() const;
  void setStaticVars(const SVInfoVec&);

  ArFunction arFuncPtr() const;

  static constexpr ptrdiff_t sharedBaseOff();
};
```

### Declaration order ###

Adhere to the following order for declarations in a struct or class definition:

1. Friend classes.
2. Nested classes, enums, typedefs. (If possible, just declare the nested
   class and define it following the enclosing class definition.)
3. Constructors, destructor.
4. Member functions, including static functions, documented and grouped
   coherently.
5. Constants and static data members.
6. *All* instance data members, regardless of accessibility.

Private member functions can be interspersed with public functions, or
relegated to a single section before the data members. However, all instance
properties *must* occur contiguously at the end of the class definition.


## Other C++ Language Features ##

Very few language features are unconditionally banned. However, if you want to
use one of the more controversial constructs such as `goto` or `operator,()`,
you'd better have a convincing argument as to why it's better than the
alternatives. C++ is a very large and complex language and we don't want to
artificially limit what developers can do, but that puts a lot of
responsibility on your shoulders.

Avoiding restrictions on useful language features (e.g., exceptions, templates,
C++11 lambdas) is a major motivating factor for maintaining our own style guide
rather than adopting an existing one.

### Namespaces ###

All HHVM code should be scoped in `namespace HPHP { /* everything */ }`. Large
submodules such as `HPHP::jit` and `HPHP::rds` may be contained in their own
namespace within `HPHP`. We often use anonymous namespaces instead of the
`static` keyword to keep symbols internal to their translation unit. This is
mostly left up to the author; just keep in mind that classes and structs,
unlike functions and variables, *must* be in an anonymous namespace in order to
be properly hidden.

Avoid `using namespace` whenever possible, especially in headers. It is
acceptable in `.cpp` files in very limited scopes (function-level or deeper) if
it will significantly aid in readability of the code that follows. `using
namespace std;` at the top of a `.cpp` is explicitly disallowed.

### Enums ###

Prefer `enum class` whenever possible. Old-style enums are generally only
acceptable if you expect that your type will be frequently used in an integer
context, such as array indexing.


## Naming ##

HHVM code adheres to the some broad naming conventions.

When the convention is left open, in general, prefer the local conventions
used in the file you are working on---e.g., in a struct whose data members all
have `m_namesLikeThis`, prefer `m_anotherNameLikeThis` to `m_this_style`, even
though the latter is found in other parts of the codebase.

### Variables ###

Use `lowerCamelCase` or `lower_case_with_underscores` for all local variables,
adhering to whichever is the discernable local convention if possible.  Static
variables (whether declared in an anonymous namespace or with the `static`
keyword) should additionally be prefixed by `s` (e.g., `s_funcVec`).  Globals,
likewise, should be prefixed by `g_` (e.g., `g_context`).

### Constants ###

All constants should be prefixed with `k` and use `CamelCase`, e.g.,
`kInvalidHandle`. Prefer `constexpr` to `const` whenever possible.

### Class data members ###

As with variables, use `lowerCamelCase` or `lower_case_with_underscores` for
all data members. Additionally, private instance members should be prefixed
with `m_` (e.g., `m_cls`, `m_baseCls`, `m_base_cls`), and all static members
should be prefixed with `s_` (e.g., `s_instance`).  Prefer to leave public
members unprefixed.

### Functions ###

We generally prefer `lowerCamelCase` for header-exposed functions, including
member functions, although we use `lower_case_with_underscores` as well (e.g.,
`hphp_session_init`), more commonly in file-local scopes.  As usual, follow the
local naming conventions of the file you are working in.

If you are modeling a class after an existing pattern, such as an STL
container, prefer to follow the appropriate conventions (e.g.,
`my_list::push_back` is preferred over `my_list::pushBack`).

### Classes ###

Classes use `UpperCamelCase`, except when modeling existing patterns like STL
containers or smart pointers.

### Namespaces ###

New namespaces should use `lowercase`---and single-word namespaces are greatly
prefered for common usage.  For longer namespaces (e.g., `vasm_detail`), use
`lower_case_with_underscores`.

### Other conventions ###

Prefer correctly capitalizing acronyms in new code (e.g., prefer `IRTranslator`
to `HhbcTranslator`). In this vein, prefer `ID` (e.g., `TransID`) to `Id`
(e.g., `FuncId`) in new code.


## Formatting ##

While consistent code formatting doesn't directly affect correctness, it makes
it easier to read and maintain. For this reason, we've come up with a set of
guidelines about how code should be formatted. There's a good chance that some
of these will conflict with your own personal preferred style, but we believe
that having a consistently easy to read codebase is more important than letting
each developer write code that he or she thinks is uncompromisingly beautiful.

Anything not specified here is left up to the judgment of the developer.
However, this document is not set in stone, so if a particular formatting issue
keeps coming up in code review it probably deserves a few lines in here.

### General rules ###

- All indentation is to be done using spaces.
- Each indentation level is 2 spaces wide.
- Lines may be no longer than 80 characters, unless absolutely required for
  some syntactic reason.
- Lines should not have any trailing whitespace. This includes blank lines at
  non-zero indentation levels; the only character on those lines should be a
  newline.

### Types and variables ###

- When declaring a variable or typedef, the `*` and `&` characters for pointer
  and reference types should be adjacent to the type, not the name (e.g.,
  `const Func*& func`).
- Limit variable declarations to one per line.

### Function signatures ###

The following function signatures are formatted properly:

```cpp
// If arguments would fit on 1 line:
inline void Func::appendParam(bool ref, const Func::ParamInfo& info) {
}

// If the arguments need to wrap, we have two accepted styles, both of which
// are OK even if the wrapping wasn't necessary:
SSATmp* HhbcTranslator::ldClsPropAddr(Block* catchBlock,
                                      SSATmp* ssaCls,
                                      SSATmp* ssaName,
                                      bool raise) {
  doSomeStuff();
}

// This style is helpful if any of the function, argument, or type names
// involved are particularly long.
SSATmp* HhbcTranslator::ldClsPropAddr(
  Block* catchBlock,
  SSATmp* ssaCls,
  SSATmp* ssaName,
  bool raise
) {
  doSomeStuff();
}
```

Always keep the type on the same line as the function name, unless it would
leave insufficient room for arguments. Do likewise with other modifying
keywords (`inline`, `static`, any attributes).

Wrapped arguments should always be aligned with the argument on the previous
line. The opening curly brace should be on the same line as the last argument,
with the exception of class constructors (see the Constructor initializer list
section). When writing function declarations in headers, include argument names
unless they add no value:

```cpp
struct Person {
  // The single string argument here is obviously the name.
  void setName(const std::string&);

  // Two string arguments, so it's not obvious what each one is without names.
  void setFavorites(const std::string& color, const std::string& animal);
};
```

### Statements ###

Conditional and loop statements should be formatted like so:

```cpp
if (vmpc() == nullptr) {
  fprintf(stderr, "whoops!\n");
  std::abort();
}
```

Note that there is a single space after the `if` keyword, no spaces between
`condition` and the surrounding parentheses, and a single space between the `)`
and the `{`. As with all blocks, the body should be one indentation level
deeper than the `if`. If the *entire* statement (condition and body) fits on
one line, you may leave it on one line, omitting the curly braces. In all
other cases, the braces are required. For example, the following are OK:

```cpp
if (obj->_count == 0) deleteObject(obj);

for (auto block : blocks) block->setParent(nullptr);
```

But these are not acceptable:

```cpp
if (veryLongVariableName.hasVeryLongFieldName() &&
    (rand() % 5) == 0) launchRocket();

if ((err = SSLHashSHA1.update(&hashCtx, &signedParams)) != 0)
  goto fail;
```

Avoid assignments in conditional expressions, unless the variable is declared
within the condition, e.g.,

```cpp
if (auto const unit = getMyUnit(from, these, args)) {
  // Do stuff with unit.
}
```

Prefer C++11 foreach syntax to explicit iterators:

```cpp
for (auto const& thing : thingVec) {
  // Do stuff with thing.
}
```

### Expressions ###

- All binary operators should have one space on each side, except for `.`,
  `->`, `.*`, and `->*` which should have zero.
- Do not include redundant parentheses unless you think the expression would be
  confusing to read otherwise. A good rule of thumb is that if you and/or your
  reviewers have to look at a chart of operator precedence to decide if the
  expression parses as expected, you probably need some extra parentheses. GCC
  or clang may suggest extra parens in certain situations; we compile with
  `-Werror` so you must always follow those guidelines.
- If an expression does not fit on one line, attempt to wrap it after an
  operator (rather than an identifier or keyword) and indent subsequent lines
  with the beginning of the current parenthesis/brace nesting level. For
  example, here are some long expressions, formatted appropriately:
```cpp
if (RuntimeOption::EvalJitRegionSelector != "" &&
    (Cfg::HHIR::RefcountOpts ||
     RuntimeOption::EvalHHITExtraOptPass) &&
    Func::numLoadedFuncs() < 600) {
  // ...
}

longFunctionName(argumentTheFirst,
                 argumentTheSecond,
                 argumentTheThird,
                 argumentTheFourth);
```

- Function calls should be formatted primarily using the previous rule. If one
  or more of the arguments to the function is very wide, it may be necessary to
  shift all the arguments down one line and align them one level deeper than
  the current scope. This is always acceptable, but is especially common when
  passing lambdas:
```cpp
m_irb->ifThen(
  [&](Block* taken) {
    gen(CheckType, Type::Int, taken, src);
  },
  [&] {
    doSomeStuff();
    lotsOfNonTrivialCode();
    // etc...
  }
);
```

### Constructor initializer lists ###

If an initializer list can be kept on a single line, it is fine to do so:

```cpp
MyClass::MyClass(uint64_t idx) : m_idx(idx) {}

MyClass::MyClass(const Func* func) : m_idx(-1) {
  // Do stuff.
}
```

Otherwise, it is always correct to format lists thusly:

```cpp
MyClass::MyClass(const Class* cls, const Func* func, const Class* ctx)
  : m_cls(cls)
  , m_func(func)
  , m_ctx(ctx)
  , m_isMyConditionMet(false)
{}

MyClass::MyClass(const Class* cls, const Func* func)
  : m_cls(cls)
  , m_func(func)
  , m_ctx(nullptr)
  , m_isMyConditionMet(false)
{
  // Do stuff.
}
```

### Namespaces ###

We don't nest namespaces very deeply, so prefer to keep the scoping to a single
line:

```cpp
namespace HPHP::jit::x64 {
///////////////////////////////////////////////////////////////////////////////

/*
 * Some nice documentation.
 */
struct SomeNiceThing {
  // some nice properties
};

///////////////////////////////////////////////////////////////////////////////
}
```

Do not increase the indentation level when entering namespace scope. Instead,
consider adding a line of forward slashes as a separator, to more clearly
delineate the namespace (this is especially useful for anonymous namespaces in
source files). This form of delineation is encouraged, but we have no strict
convention for its formatting (you'll see 70- or 79- or 80-character
separators, with or without an extra newline between it and the braces, etc.).


## Comments ##

All public and private APIs in headers should be documented in detail. Names
and notions which are not obvious (e.g., "persistent" or "simple") should be
explained. Preconditions and postconditions should be noted.

Inline code comments are encouraged for complex logic, but their density is
left up to the author. Rather than summarizing/paraphrasing what your code is
doing, focus on explaining what overarching goal the code is achieving and/or
why that goal is necessary or desirable.

### Comment style ###

Here are some comment styles we use or avoid:

```cpp
// This style of comment is the most common for relatively short inline
// comments. It's fine if it's multi-line.
//
// It's also fine if it has line breaks. The extra newline aids readability in
// this case.

/*
 * This style of comment is the right one to use for struct/function
 * documentation. Prefer one star on the opening line, as opposed to the
 * doxygen standard of two.
 *
 * This is also sometimes used for inline code comments, although the // style
 * makes it easier to comment out blocks of code.
 */

struct ClassLikeThing {
  std::vector<const Func*> methods; // This is fine for short annotations.

  /* This is also ok, though try not to mix and match too much in a file. */
  std::vector<const ClassLikeThing*> parents;
};

/* Don't write multiline comments where some lines are missing their prefix.
   This is pretty weird. */
```

Try to use complete sentences in all but the shortest of comments. All comments
should be flowed to 79 characters in width.

### Separators ###

Delineate sections of code with a line of forward slashes. There is no strict
convention, but prefer lines of slashes to other delineators (e.g., `/*****/`,
five newlines, ASCII-art cartoon characters).

### File copyright ###

All files must begin with a copyright/license notice. For files created by
Facebook employees, the following should be used:

```cpp
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-201x Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

// File contents start here.
```

We do not require copyright assignment for external contributions, so
non-Facebook contributors should include their own header. The exact contents
are up to you, as long as the license is compatible with the PHP license.
