
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) after a filtering operation has been
applied to each value in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
namespace HH\Asio;

function vf<Tk, T>(
  KeyedTraversable<Tk, T, mixed> $inputs,
  (function(T): Awaitable<bool>) $callable,
): Awaitable<Vector<T>>;
```




This function is similar to [` Vector::filter() `](/apis/Classes/HH/Vector/filter/), but the filtering of the
values is done using [` Awaitable `](/apis/Classes/HH/Awaitable/)s.




This function is called ` vf ` because we are returning a `` v ``ector, and
we are doing a ``` f ```iltering operation.




` $callable ` must return an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` bool ``.




The values in the [` Vector `](/apis/Classes/HH/Vector/) of the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) are not available
until you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` T, mixed> $inputs `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) of values to map.
+ ` (function(T): Awaitable<bool>) $callable ` - The callable containing the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation to
  apply to `` $inputs ``.




## Returns




* [` Awaitable<Vector<T>> `](/apis/Classes/HH/Awaitable/) - An [` Awaitable `](/apis/Classes/HH/Awaitable/) of [` Vector `](/apis/Classes/HH/Vector/) after the filtering operation has been
  applied to the values in  `` $inputs ``.




## Examples




``` basic-usage.hack
<<__EntryPoint>>
async function basic_usage_main(): Awaitable<void> {
  $times = ImmVector {
    100000000, // Sat, 03 Mar 1973 09:46:40
    200000000, // Mon, 03 May 1976 19:33:20
    300000000, // Thu, 05 Jul 1979 05:20:00
    400000000, // Sat, 04 Sep 1982 15:06:40
    500000000, // Tue, 05 Nov 1985 00:53:20
    600000000, // Thu, 05 Jan 1989 10:40:00
    700000000, // Sat, 07 Mar 1992 20:26:40
    800000000, // Tue, 09 May 1995 06:13:20
    900000000, // Thu, 09 Jul 1998 16:00:00
    1000000000, // Sun, 09 Sep 2001 01:46:40
  };

  // Similar to $times->filter(...)
  // But awaits the awaitable result of the callback
  // rather than using it directly
  $saturdays =
    await \HH\Asio\vf($times, async ($time) ==> (\gmdate('w', $time) === '6'));

  foreach ($saturdays as $time) {
    echo \gmdate('r', $time), "\n";
  }
}
```
<!-- HHAPIDOC -->
