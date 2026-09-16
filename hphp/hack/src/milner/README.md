# Milner

Milner generates Hack programs from templates using an independent model of
typing and subtyping. The typechecker and runtime are test oracles, not filters
used to choose which generated programs to retain.

```
buck run @//mode/opt-clang //hphp/hack/src/milner:milner -- TEMPLATE --seed 42
```

`TYPE#1`, `SUBTYPE#1`, and `expr#1` request a type, a subtype, and an
inhabitant with the same integer identifier. Auxiliary definitions are appended
to the generated program. Templates supply the property under test and its legal
typing contexts.

Every generated value type has an inhabited subtype. Immediate inhabitance is a
pure structural check: selecting an inhabited subtype does not construct an
expression or consume randomness merely to decide whether a witness exists.

Expression construction carries the same generation context and nominal
environment through recursive witnesses.

Every identifier has one generated type and environment, including identifiers
that occur only in expressions. Their auxiliary declarations are retained.
Repeated placeholders share their replacement; longer numeric identifiers are
substituted first so `#1` cannot consume the prefix of `#10`.

`ALIAS_TYPE#1` constrains that identifier's type and subtype choices everywhere
to legal alias right-hand sides. It excludes direct type-constant references;
type constants nested in other type constructors remain available.
