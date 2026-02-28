
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an int for the lower (last) 64 bits of an md5 hash of a string




``` Hack
namespace HH;

function non_crypto_md5_lower(
  string $str,
): int;
```




The MD5 hash, usually presented as a hex value, is taken as big endian, and
this int result is the signed counterpart to the unsigned lower 64 bits.




This function and the _upper version are generally only intended for
legacy use cases in which an MD5 hash is used to compute a number
of 64 bits or less. These functions are faster and prettier than calling
unpack+substr+md5/raw or hexdec+substr+md5. Note that hexdec converts
to floating point (information loss) for some 64-bit unsigned values.




The faster and quite effective xxhash64 is generally recommended for
non-crypto hashing needs when no backward compatibility is needed.




## Parameters




+ ` string $str `




## Returns




* ` int `
<!-- HHAPIDOC -->
