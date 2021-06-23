<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private\_IO {
  use namespace HH\Lib\{C, Dict, Str, Vec};

  <<__EntryPoint>>
  function generate_intersection_interfaces(): void {
    // Map these to powers of two, so we can later use a bitmask
    $bases = vec[
      'Closeable',
      'Seekable',
      'Read',
      'Write',
      'FD',
    ]
      |> Dict\flip($$)
      |> Dict\map($$, $vec_idx ==> 2 ** $vec_idx as int);

    for ($i = 3; $i < (2 ** C\count($bases)); $i++) {
      // $i is a bitmask that represents:
      // - the current interface
      // - the parents: for each set bit, turn it off. That's a parent.
      //
      // For example, the parents of 111 are 011, 101, and 110.
      //
      // If we have ^0*10*$, we have a direct child of 'Handle', not an
      // intersection (a.k.a. composite/intermediate) interface.
      if (Str\trim((string)$i, '0') === '1') {
        continue;
      }
      $active = Dict\filter($bases, $bit ==> ($i & $bit) === $bit);
      $parents = Dict\map(
        $active,
        $this_bit ==> Dict\filter($active, $bit ==> $bit !== $this_bit)
          |> Vec\keys($$)
          |> Str\join($$, '').'Handle',
      );
      if (C\count($parents) === 1) {
        continue;
      }
      \printf(
        "interface %sHandle extends %s {}\n",
        Str\join(Vec\keys($active), ''),
        Str\join($parents, ', '),
      );
    }
  }

}

namespace HH\Lib\IO {

  // Generated with the above function, then `hackfmt`

  interface CloseableSeekableHandle extends SeekableHandle, CloseableHandle {}
  interface CloseableReadHandle extends ReadHandle, CloseableHandle {}
  interface SeekableReadHandle extends ReadHandle, SeekableHandle {}
  interface CloseableSeekableReadHandle
    extends SeekableReadHandle, CloseableReadHandle, CloseableSeekableHandle {}
  interface CloseableWriteHandle extends WriteHandle, CloseableHandle {}
  interface SeekableWriteHandle extends WriteHandle, SeekableHandle {}
  interface CloseableSeekableWriteHandle
    extends
      SeekableWriteHandle,
      CloseableWriteHandle,
      CloseableSeekableHandle {}
  interface ReadWriteHandle extends WriteHandle, ReadHandle {}
  interface CloseableReadWriteHandle
    extends ReadWriteHandle, CloseableWriteHandle, CloseableReadHandle {}
  interface SeekableReadWriteHandle
    extends ReadWriteHandle, SeekableWriteHandle, SeekableReadHandle {}
  interface CloseableSeekableReadWriteHandle
    extends
      SeekableReadWriteHandle,
      CloseableReadWriteHandle,
      CloseableSeekableWriteHandle,
      CloseableSeekableReadHandle {}
  interface CloseableFDHandle extends FDHandle, CloseableHandle {}
  interface SeekableFDHandle extends FDHandle, SeekableHandle {}
  interface CloseableSeekableFDHandle
    extends SeekableFDHandle, CloseableFDHandle, CloseableSeekableHandle {}
  interface ReadFDHandle extends FDHandle, ReadHandle {}
  interface CloseableReadFDHandle
    extends ReadFDHandle, CloseableFDHandle, CloseableReadHandle {}
  interface SeekableReadFDHandle
    extends ReadFDHandle, SeekableFDHandle, SeekableReadHandle {}
  interface CloseableSeekableReadFDHandle
    extends
      SeekableReadFDHandle,
      CloseableReadFDHandle,
      CloseableSeekableFDHandle,
      CloseableSeekableReadHandle {}
  interface WriteFDHandle extends FDHandle, WriteHandle {}
  interface CloseableWriteFDHandle
    extends WriteFDHandle, CloseableFDHandle, CloseableWriteHandle {}
  interface SeekableWriteFDHandle
    extends WriteFDHandle, SeekableFDHandle, SeekableWriteHandle {}
  interface CloseableSeekableWriteFDHandle
    extends
      SeekableWriteFDHandle,
      CloseableWriteFDHandle,
      CloseableSeekableFDHandle,
      CloseableSeekableWriteHandle {}
  interface ReadWriteFDHandle
    extends WriteFDHandle, ReadFDHandle, ReadWriteHandle {}
  interface CloseableReadWriteFDHandle
    extends
      ReadWriteFDHandle,
      CloseableWriteFDHandle,
      CloseableReadFDHandle,
      CloseableReadWriteHandle {}
  interface SeekableReadWriteFDHandle
    extends
      ReadWriteFDHandle,
      SeekableWriteFDHandle,
      SeekableReadFDHandle,
      SeekableReadWriteHandle {}
  interface CloseableSeekableReadWriteFDHandle
    extends
      SeekableReadWriteFDHandle,
      CloseableReadWriteFDHandle,
      CloseableSeekableWriteFDHandle,
      CloseableSeekableReadFDHandle,
      CloseableSeekableReadWriteHandle {}
}
