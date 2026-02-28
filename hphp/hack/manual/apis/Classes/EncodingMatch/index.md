---
title: EncodingMatch
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Result of detecting the encoding of an array of bytes




## Interface Synopsis




``` Hack
class EncodingMatch {...}
```




### Public Methods




+ [` ->getConfidence(): int `](/apis/Classes/EncodingMatch/getConfidence/)\
  Gets the confidence number of the encoding match

+ [` ->getEncoding(): string `](/apis/Classes/EncodingMatch/getEncoding/)\
  Gets the name of the detected encoding

+ [` ->getLanguage(): string `](/apis/Classes/EncodingMatch/getLanguage/)\
  Gets a rough guess at the language of the encoded bytes

+ [` ->getUTF8(): string `](/apis/Classes/EncodingMatch/getUTF8/)\
  Gets the UTF-8 encoded version of the encoded byte array

+ [` ->isValid(): bool `](/apis/Classes/EncodingMatch/isValid/)\
  Checks if the encoding match succeeded

<!-- HHAPIDOC -->
