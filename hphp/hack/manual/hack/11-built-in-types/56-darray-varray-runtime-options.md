# Darray Varray Runtime Options

As of [HHVM 4.103](https://hhvm.com/blog/2021/03/31/hhvm-4.103.html), `darray` / `varray` are aliased to `dict` / `vec` respectively. Use [Hack arrays](/hack/arrays-and-collections/vec-keyset-and-dict).

:::warning Warning
These runtime options are a migrational feature. This means that they come and go when new hhvm versions are released. Before relying on them, it is recommended to run the given example code. If this does not raise a "Hack Arr Compat Notice" this option is not available in your version of HHVM.
:::

If you notice that an option doesn't apply anymore and you are running a very modern version of HHVM, please open an issue or pull request against this repository. We'll mark the EOL date of that given runtime option in the documentation. We thank you in advance.

The [runtime options](/hack/arrays-and-collections/introduction) were briefly introduced in the article on [arrays](/hack/arrays-and-collections/introduction). This article builds upon the information given there.

You can get a list of the runtime options that your current hhvm recognizes from this script.
This relies on the settings being in your `server.ini`.
The output will look something like this.

```hack
function get_all_runtime_options(
): dict<string, shape(
  'global_value' => string,
  'local_value' => string,
  'access' => string,
)> {
  return \ini_get_all()
    |> Dict\filter_keys($$, $name ==> Str\contains($name, 'hack_arr'));
}

<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  foreach (get_all_runtime_options() as $name => $values) {
    echo Str\format(
      "%s> global_value(%s), local_value(%s), access(%s)\n",
      Str\pad_right($name, 60, '-'),
      $values['global_value'],
      $values['local_value'],
      $values['access'],
    );
  }
}
```

*Example output (HHVM 4.115)*

```
hhvm.hack_arr_is_shape_tuple_notices------------------------> global_value(), local_value(), access(4)
hhvm.hack_arr_dv_arrs---------------------------------------> global_value(1), local_value(1), access(4)
hhvm.hack_arr_dv_arr_var_export-----------------------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_cast_marked_array_notices--------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_compact_serialize_notices--------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_serialize_notices----------------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_is_vec_dict_notices--------------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_intish_cast_notices--------------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_fb_serialize_hack_arrays_notices-------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_check_compare--------------------------> global_value(), local_value(), access(4)
hhvm.hack_arr_compat_notices--------------------------------> global_value(), local_value(), access(4)
```

An important note: These settings will not work when you set them at runtime using ini_set(). You must set these in your configuration file or pass them in using the `-dsettinghere=valuehere` command line argument when invoking your script from the command line.

## Check implicit varray append

Fullname: hhvm.hack_arr_compat_check_implicit_varray_append

:::warning Warning
This option was removed in HHVM 4.64. It is now always a fatal error.
:::

Before HHVM 4.64, this setting will raise a notice under the following condition.
If it does not raise a warning, this option is not available in your version of hhvm.

```hack no-extract

<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  using _Private\print_short_errors();

  $varray = varray[
    'HHVM',
    'HACK',
  ];

  $varray[2] = <<<EOF
Writing to the first unused key of a varray.
Varray's behave differently here than vecs.
EOF;
}
```

*Output (before HHVM 4.64)*

```
E_NOTICE "Hack Array Compat: Implicit append to varray" in file "hack_arr_compat_check_implicit_varray_append.php" at line 19
```

**(fatal error in HHVM 4.64 or newer)**

A `vec<_>` does not support implicitly appending. You can only append using an empty subscript operator `$x[] = ''` and update using a keyed subscript operator `$x[2] = ''`. The runtime will throw when you use the updating syntax in order to append. `'OutOfBoundsException' with message 'Out of bounds vec access: invalid index 2'`.

A `varray<_>` will, before HHVM 4.64, accept you implicitly appending a key. It will remain a `varray<_>`. This is the only case where writing to a non existent index in a `varray<_>` will not cause the `varray<_>` to escalate to a `darray<_, _>`. More information about array escalation can be found below.

## Check varray promote

Fullname: hhvm.hack_arr_compat_check_varray_promote

:::warning Warning
This option was removed in HHVM 4.64. It is now always a fatal error.
:::

Before HHVM 4.64, this setting will raise a notice under the following condition.
If it does not raise a warning, this option is not available in your version of hhvm.

```hack no-extract
<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  using _Private\print_short_errors();

  $varray = varray[
    'HHVM',
    'HACK',
  ];

  $varray[3] = <<<EOF
Writing to a key that is not already in use nor the first unused key.
A vec<_> would throw an exception here.
EOF;

  $varray = varray[
    'HHVM',
    'HACK',
  ];

  /*HH_IGNORE_ERROR[4135] This is banned in strict mode, but needs to be illustated.*/
  unset($varray[0]);
  // Using unset on an index that is not the greatest index.

  $varray = varray[
    'HHVM',
    'HACK',
  ];

  /*HH_IGNORE_ERROR[4324] This is banned in Hack, but needs to be illustated.*/
  $varray['string'] = <<<EOF
Writing to a string key in a will escalate it to a darray<_, _>.
A vec would throw an exception here.
EOF;
}
```

*Output (before HHVM 4.64)*

```
E_NOTICE "Hack Array Compat: varray promoting to darray: out of bounds key 3" in file "hack_arr_compat_check_varray_promote.php" at line 19
E_NOTICE "Hack Array Compat: varray promoting to darray: removing key" in file "hack_arr_compat_check_varray_promote.php" at line 30
E_NOTICE "Hack Array Compat: varray promoting to darray: invalid key: expected int, got string" in file "hack_arr_compat_check_varray_promote.php" at line 39
```

**(fatal error in HHVM 4.64 or newer)**

These situations are sadly very common in grandfathered PHP code.

The first situation, writing to a key out of bounds, is not permitted on a `vec<_>`. It throws and `OutOfBoundsException`. A `vec<_>` will always maintain the keys 0, 1, 2, ... and will therefore have to refuse to create the new index on the fly.

There are two distinct intents that the programmer may have had when writing this code.

- The keys are actually useful data.
- The keys are meant to be indexes 0, 1, 2 and the programmer assumed that he or she was writing in-bounds.

The first case is usually pretty easy to fix. If it looks like the keys are userids, timestamps, or alike, `varray<_>` isn't the right type. Migrate the code to use `darray<_, current_value_type>`. You'll have to figure out the keytype from context.

The second case is far less easy to give a clear fix for.

- Chances are that there is a nearby `C\count()` doing a bounds check that might be defective.
- Is the array being filled out of order? Are all the indexes between 0 and the greatest index used after this procedure? You might be tempted to make the fill happen in order, but that will change the order that the elements are iterated over in a foreach.

The second situation, calling unset on an element of a `varray<_>`, can have multiple intends too.

- If the T is a nullable type, the programmer might have meant to write `null` to the index. This is more common in code written before hhvm4.
- The programmer does not care about the keys. The array is merely a meant to be used as a `KeyedContainer<not_important, T>` and he or she just meant to remove the value from the `KeyedContainer<_, _>`.
- The programmer intended to unset the last index.

The first case is most likely a confusion caused by a removed behavior of all legacy arrays. Before hhvm 4 accessing an key that wasn't present would log a notice and return null. An unset on an array would under these circumstances act very similarly to explicitly setting to value to null. This is however quite tricky to do right if this array is being passed around the program a lot. An unset key is actually removed from the array. This means that `C\contains_key()` will return `false`, `idx()` will return its default argument, and `??` will evaluate to the RHS. However, explicitly setting the value to `null` does not remove the key from the array. This means that `C\contains_key()` returns `true`, `idx()` will return the `null`, but `??` will be unaffected.

The second usecase is not met by Hack arrays. There is no `Container<_>` type that allows you to append to the end and remove things by index (except for `keyset<_>`, but that has a constraint value type). You can however emulate this behavior using a `varray<_>` or `vec<_>`. Removing the first key can be done using `Vec\drop($x, 1)`. Removing the last key can be done using `\array_pop()` <span data-nosnippet class="fbonly apiAlias">`C\fb\pop_back`</span>. Removing a key from the middle can be done with the slightly unwieldy `Vec\filter_with_key($x, ($key, $_) ==> $key === 1)`. All of these will rekey the array. Any values after the key will be shifted down. This does however have a computational complexity of `O(n)`. If you need to remove a lot of keys from the middle that are next to each other, use `Vec\slice()` to save some resources. If you need to remove a ton of arbitrary keys, at different points of your function it might be better to `dict($x)`, unset on the `dict<_, _>` and rekey it back to a `varray<_>` using `varray()` or `array_values()` depending on your hhvm version.

The third usecase used to be valid Hack. Unsetting the last index of a `varray<_>` or `vec<_>` was allowed and acted like an `\array_pop()`<span data-nosnippet class="fbonly apiAlias">`C\fb\pop_back`</span>. This will currently not generate a warning, but it is unclear to me if this will continue to be allowed. The typechecker already raises a typeerror when you use unset on a non dictionary/hashmap like array type.

The third situation, writing to a string key, is always a mistake.

If this is a string literal, the actual type is most likely `darray<_, _>`. If this is an intergral string coming from an untyped function, it is worth investigating casting the value to an int.

## Runtime typetests of shapes and tuples

In HHVM 4.102 or older, shapes and tuples were implemented with `darray` and
`varray`. In HHVM 4.103 and newer, they are `dict` and `vec`;
this means that the following checks would fail before HHVM 4.103, but now
pass&mdash;for this reason, in HHVM 4.102 or older,
the `hhvm.hack_arr_is_shape_tuple_notices`
runtime option could be used to raise notices for these type tests:

```hack
$_ = dict[] is shape();
$_ = vec[42] is /*tuple*/(int);
```

*Output (before HHVM 4.103)*

```
Notice: Hack Array Compat: dict is shape in /home/example/hack_arr_is_shape_tuple_notices.hack on line 10

Notice: Hack Array Compat: vec is tuple in /home/example/hack_arr_is_shape_tuple_notices.hack on line 11
```

## Check array key cast

Fullname: hhvm.hack_arr_compat_check_array_key_cast

:::warning Warning
This option was removed in HHVM 4.66. It is now always a fatal error.
:::

Before HHVM 4.66, this setting will raise a notice under the following condition.
If it does not raise a warning, this option is not available in your version of hhvm.

```hack no-extract
<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  using _Private\print_short_errors();

  $varray = varray[];

  /*HH_IGNORE_ERROR[4324]*/
  $varray[1.1] = 'A float?!?';
  /*HH_IGNORE_ERROR[4324]*/
  $varray[true] = 'A bool?!?';
  /*HH_IGNORE_ERROR[4324]*/
  $varray[null] = 'null?!?';

  $darray = darray[];

  /*HH_IGNORE_ERROR[4371]*/
  $darray[1.1] = 'A float?!?';
  /*HH_IGNORE_ERROR[4371]*/
  $darray[true] = 'A bool?!?';
  /*HH_IGNORE_ERROR[4371]*/
  $darray[null] = 'null?!?';

}
```

*Output (before HHVM 4.66)*

```
E_NOTICE "Hack Array Compat: Implicit conversion of double to array key" in file "hack_arr_compat_check_array_key_cast.php" at line 17
E_NOTICE "Hack Array Compat: Implicit conversion of bool to array key" in file "hack_arr_compat_check_array_key_cast.php" at line 19
E_NOTICE "Hack Array Compat: Implicit conversion of null to array key" in file "hack_arr_compat_check_array_key_cast.php" at line 21
E_NOTICE "Hack Array Compat: Implicit conversion of double to array key" in file "hack_arr_compat_check_array_key_cast.php" at line 26
E_NOTICE "Hack Array Compat: Implicit conversion of bool to array key" in file "hack_arr_compat_check_array_key_cast.php" at line 28
E_NOTICE "Hack Array Compat: Implicit conversion of null to array key" in file "hack_arr_compat_check_array_key_cast.php" at line 30
```

**(fatal error in HHVM 4.66 or newer)**

A `vec<_>` and a `dict<_, _>` only allow `arraykey` keys.
Because of legacy, the (d/v)array family needed to support non arraykey keys being set and read from.
When you set `$varray[true] = 4;`, before HHVM 4.66, hhvm will cast your `true` to a valid arraykey `1`.
The rules of casting were as follows:

- floats are cast to ints using an `(int)` cast.
- `true` becomes 1 and `false` becomes 0.
- `null` becomes empty string.

Deciding on the best course of action relies on context.
If the value is coming from an untyped function, it is worth investigating if the returned type might have been a mistake.
Keep in mind that a function that reaches the closing `}` before hitting a `return` statement returns `null`.

If you want this error to go away, but you'd like to keep the current behavior (not recommended)
you can use `HH\array_key_cast()`.
