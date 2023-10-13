<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */



interface Dummy {
  public function format_0x25(): string;
  public function format_0x2e(): Dummy;
  public function format_f(float $s): string;
  public function format_s(string $s): string;
  public function format_0x24(): Dummy2;
}

interface Dummy2 {
  public function format_wild(int $i): Dummy;
}

function f(\HH\FormatString<Dummy> $fs, mixed ...$args): void {
}

function _(): void {
  f('%..$^...s %%', 42, 'foo');
  f('%f', 0.1);
  f('%' . 's', 0.1);
}
