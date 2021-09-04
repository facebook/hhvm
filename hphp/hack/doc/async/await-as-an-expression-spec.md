For a summary, see the [await-as-an-expression](https://docs.hhvm.com/hack/asynchronous-operations/await-as-an-expression) docs.

The guiding principle of “unconditionally consumed with the statement” is to prevent computation from being thrown away. Since we could run the awaits before the statement, if they turn out to not be consumed, it will result in waste. We intentionally ignore the possibility of Exceptions being thrown for the definition of “conditional”.

For the position to be considered “unconditionally consumed” we require all parents of the await expression until the closest statement pass this check.

Valid positions:

* ConditionalExpression: Only first position is allowed
    * `(await $yes) ? (await $no) : (await $no)`
* FunctionCallExpression: Any position is allowed as long as receive isn't conditionally executed. Only the receiver is allowed if the receiver is conditional.
    * `(await $yes)->foo(await $yes, await $yes)`
    * `(await $yes)?->foo(await $no, await $no)`
* SafeMemberSelectionExpression: Only the object is allowed.
    * `(await $yes)?->(await $no)`
* Allow any valid expression position of all of the following expressions:
    * CastExpression: `(<token>)(await $yes)`
    * MemberSelectionExpression: `(await $yes)->(await $yes)`
    * ScopeResolutionExpression: `(await $yes)::(await $yes)`
    * IsExpression: `(await $yes) is Int`
    * AsExpression: `(await $yes) as Int`
    * NullableAsExpression: `(await $yes) ?as Int`
    * EmptyExpression: `empty(await $yes)`
    * IssetExpression: `isset(await $yes)`
    * ParenthesizedExpression: `(await $yes)`
    * BracedExpression: `{await $yes}`
    * EmbeddedBracedExpression: `“{await $yes}"`
    * CollectionLiteralExpression: `Map { await $yes => await $yes }` or `Vector { await $yes }`
    * ObjectCreationExpression, ConstructorCall: `new (await $yes)(await $yes)`
    * ShapeExpression, FieldInitializer: `shape('key'`` => await $yes)`
    * TupleExpression: `tuple(await $yes, await $yes)`
    * ArrayCreationExpression: `array(await $yes => await $yes)`
    * ArrayIntrinsicExpression: `[await $yes => await $yes]`
    * DarrayIntrinsicExpression: `darray[await $yes => await $yes]`
    * VarrayIntrinsicExpression: `varray[await $yes]`
    * DictionaryIntrinsicExpression: `dict[await $yes => await $yes]`
    * KeysetIntrinsicExpression: `keyset[await $yes]`
    * VectorIntrinsicExpression: `vec[await $yes]`
    * ElementInitializer: `await $yes => await $yes`
    * SubscriptExpression: `(await $yes)[await $yes]`
    * EmbeddedSubscriptExpression: `{(await $yes)[await $yes]}`
    * YieldExpression: `yield (await $yes)`
    * SyntaxList, ListItem: `await $yes, await $yes`
* PrefixUnaryExpression | PostfixUnaryExpression | DecoratedExpression
    * `!`: `!(await $yes)`
    * `~`: `~(await $yes)`
    * `+`: `+(await $yes)`
    * `-`: `-(await $yes)`
    * `@`: `@(await $yes)`
    * `clone`: `clone (await $yes)`
    * `print`: `print (await $yes)`
* BinaryExpression
    * Binary operators that only allow await in the left position:
        * `AND`: `(await $yes) AND (await $no)`
        * `OR`: `(await $yes) OR (await $no)`
        * `||`: `(await $yes) || (await $no)`
        * `&&`: `(await $yes) && (await $no)`
        * `?:`: `(await $yes) ?: (await $no)`
        * `??`: `(await $yes) ?? (await $no)`
    * Binary operators that do assignment:
        * `=`: `(await $no) = (await $yes)`
        * `|=`: `(await $no) |= (await $yes)`
        * `+=`: `(await $no) += (await $yes)`
        * `*=`: `(await $no) *= (await $yes)`
        * `**=`: `(await $no) **= (await $yes)`
        * `/=`: `(await $no) /= (await $yes)`
        * `.=`: `(await $no) .= (await $yes)`
        * `-=`: `(await $no) -= (await $yes)`
        * `%=`: `(await $no) %= (await $yes)`
        * `^=`: `(await $no) ^= (await $yes)`
        * `&=`: `(await $no) &= (await $yes)`
        * `<<=`: `(await $no) <<= (await $yes)`
        * `>>=`: `(await $no) >>= (await $yes)`
    * Null Coalescing Assignment is both, so it doesn't allow either:
        * `??=`: `(await $no) ??= (await $no)`
    * Binary operators that allow await in both positions:
        * `+`: `(await $yes) + (await $yes)`
        * `-`: `(await $yes) - (await $yes)`
        * `*`: `(await $yes) * (await $yes)`
        * `/`: `(await $yes) / (await $yes)`
        * `**`: `(await $yes) ** (await $yes)`
        * `===`: `(await $yes) === (await $yes)`
        * `<`: `(await $yes) < (await $yes)`
        * `>`: `(await $yes) > (await $yes)`
        * `==`: `(await $yes) == (await $yes)`
        * `%`: `(await $yes) % (await $yes)`
        * `.`: `(await $yes) . (await $yes)`
        * `!=`: `(await $yes) != (await $yes)`
        * `<>`: `(await $yes) <> (await $yes)`
        * `!==`: `(await $yes) !== (await $yes)`
        * `<=`: `(await $yes) <= (await $yes)`
        * `<=>`: `(await $yes) <=> (await $yes)`
        * `>=`: `(await $yes) >= (await $yes)`
        * `&`: `(await $yes) & (await $yes)`
        * `|`: `(await $yes) | (await $yes)`
        * `<<`: `(await $yes) << (await $yes)`
        * `>>`: `(await $yes) >> (await $yes)`
        * `^`: `(await $yes) ^ (await $yes)`
* Pipe BinaryExpression (`|>`): Since we disallow dependent awaits in a single statement (or concurrent block), we need to disallow awaits in pipe operators that de-sugar to nested awaits. The simple rule to disallow this: we treat the `$$` as an `await` if the left side contains an `await`.
    * Disallowed: `(await $x) |> (await $$)` de-sugars into `(await (await $x))`
    * Allowed: `$x |> (await $$)` de-sugars into `(await $x)`
    * Allowed: `(await $x) |> f($$)` de-sugars into `f(await $x)`
    * Allowed: `(await $x) |> (f($$) + await $y)` de-sugars into `f(await $x) + await $y`
    * Disallowed: `(await $x) |> f($$) |> await g($$)` de-sugars into `await g(f(await $x))`
    * Disallowed: `(await $x) |> $y ?? $$` de-sugars into `$y ?? (``await $x)`
* Statement position:
    * ExpressionStatement: `await $yes;`
    * ReturnStatement: `return await $yes;`
    * UnsetStatement: `unset($a[await $yes]);`
    * EchoStatement: `echo (await $yes);`
    * PrintStatement: `print (await $yes);`
    * IfStatement: `if (await $yes) { await $yes_but_new_statement; }`
    * ThrowStatement: `throw (await $yes);`
    * SwitchStatement: `switch (await $yes) { ... }`
    * ForeachStatement: `foreach ((await $yes) [await] as ... ) { ... }`
    * ForStatement: `for (await $yes; await $no; await $no) { ... }`
* Disallowed in all other unmentioned positions.
