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
