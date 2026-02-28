---
title: EncodingDetector
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Guesses the encoding of an array of bytes in an
unknown encoding




http://icu-project.org/apiref/icu4c/ucsdet_8h.html




## Interface Synopsis




``` Hack
class EncodingDetector {...}
```




### Public Methods




+ [` ->__construct(): void `](/apis/Classes/EncodingDetector/__construct/)\
  Creates an encoding detector

+ [` ->detect(): EncodingMatch `](/apis/Classes/EncodingDetector/detect/)\
  Returns an EncodingMatch object containing the best guess
  for the encoding of the byte array

+ [` ->detectAll(): varray<EncodingMatch> `](/apis/Classes/EncodingDetector/detectAll/)\
  Returns an array of EncodingMatch objects containing all guesses
  for the encoding of the byte array

+ [` ->setDeclaredEncoding(string $encoding): void `](/apis/Classes/EncodingDetector/setDeclaredEncoding/)\
  If the user provided an encoding in metadata
  (like an HTTP or XML declaration),
  this can be used as an additional hint to the detector

+ [` ->setText(string $text): void `](/apis/Classes/EncodingDetector/setText/)\
  Sets the input byte array whose encoding is to be guessed

<!-- HHAPIDOC -->
