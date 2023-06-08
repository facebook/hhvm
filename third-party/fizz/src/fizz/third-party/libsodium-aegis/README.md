# Libsodium-aegis

/*
 * ISC License
 *
 * Copyright (c) 2013-2023
 * Frank Denis <j at pureftpd dot org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


Upstream source: https://github.com/jedisct1/libsodium

- [Dec 9, 2022: Add AEGIS-128L software support](https://github.com/jedisct1/libsodium/commit/11d2fa5bb0140e463a38fd875cbaac00e9f289d9)
- [Dec 9, 2022: Add AEGIS-256 software support](https://github.com/jedisct1/libsodium/commit/408125a72b5cbf0ccd9e478dae6b90f8737d3ee7)
- [Mar 12, 2023: AEGIS/ARM: help the compiler emit eor3 instructions on recent ARM CPUs](https://github.com/jedisct1/libsodium/commit/66a68f0947ae81ee05958bf710bb5df589b39c8b)


### Changes we made in this directory

* Exported symbol renaming


### Testing

`[fbsource/fbcode] $buck2 test fizz/crypto/aead/test:aegisciphers`
