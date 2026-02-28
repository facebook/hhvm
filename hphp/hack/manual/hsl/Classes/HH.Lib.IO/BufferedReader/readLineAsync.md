
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read until the platform end-of-line sequence is seen, or EOF is reached




``` Hack
public function readLineAsync(): Awaitable<?string>;
```




On current platforms, this is always ` \n `; it may have other values on other
platforms in the future, e.g. `` \r\n ``.




The newline sequence is read (so won't be returned by other calls), but is not
included in the return value.




+ Returns null if the end of file is reached with no data.
+ Returns a string otherwise




Some illustrative edge cases:

* ` '' ` is considered a 0-line input
* ` 'foo' ` is considered a 1-line input
* ` "foo\nbar" ` is considered a 2-line input
* ` "foo\nbar\n" ` is also considered a 2-line input




## Returns




- [` Awaitable<?string> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
