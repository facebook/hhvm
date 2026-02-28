
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Generates a backtrace for $awaitable




``` Hack
namespace HH\Asio;

function backtrace<T>(
  Awaitable<T, mixed> $awaitable,
  int $options = \DEBUG_BACKTRACE_PROVIDE_OBJECT,
  int $limit = 0,
): varray<dict<string, mixed>, darray>;
```




Following conditions must be met to produce non-empty backtrace:

+ $awaitable has not finished yet (i.e. has_finished($awaitable) === false)
+ $awaitable is part of valid scheduler context
  (i.e. $awaitable->getContextIdx() > 0)
  If either condition is not met, backtrace() returns empty array.




## Parameters




* [` Awaitable<T, `](/apis/Classes/HH/Awaitable/)`` mixed> $awaitable `` - Awaitable, to take backtrace from.
* ` int $options = \DEBUG_BACKTRACE_PROVIDE_OBJECT ` - bitmask of the following options:
  DEBUG_BACKTRACE_PROVIDE_OBJECT
  DEBUG_BACKTRACE_PROVIDE_METADATA
  DEBUG_BACKTRACE_IGNORE_ARGS
* ` int $limit = 0 ` - the maximum number of stack frames returned.
  By default (limit=0) it returns all stack frames.




## Returns




- ` array ` - - Returns an array of associative arrays.
  See debug_backtrace() for detailed format description.
<!-- HHAPIDOC -->
