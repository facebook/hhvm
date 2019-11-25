#!/usr/bin/env hhvm
/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HHVM\Packaging;

final class ThirdPartyBinaryNotices {
  public static function getKnownProjects(
  ): dict<string, ?(string, keyset<string>)> {
    return dict[
      // Special cases:
      'CMakeFiles' => null,
      'util' => null,
      'forks' => null,
      '.' => null,

       // Projects
      'brotli' => tuple('Brotli', keyset['brotli/src/LICENSE']),
      'boost' => tuple('Boost', keyset['boost/boost/LICENSE_1_0.txt']),
      'double-conversion' =>
        tuple('double-conversion', keyset['double-conversion/LICENSE']),
      'fastlz' => tuple('FastLZ', keyset['fastlz/src/LICENSE']),
      'fatal' => null, // First-party, i.e. Facebook project
      'fb303' => null, // first-party
      'fizz' => null, // first-party
      'fmt' => tuple('fmt', keyset['fmt/fmt/LICENSE.rst']),
      'folly' => null, // first-party
      'libafdt' => null, // first-party
      'libmbfl' => tuple(
        'Libmbfl',
        keyset[
          'forks/libmbfl/DISCLAIMER',
          'forks/libmbfl/LICENSE',
        ],
      ),
      'libsodium' => tuple('libsodium', keyset['libsodium/libsodium/LICENSE']),
      'libsqlite3' => null, // has a blessing instead of legal notices
      'libzip' => tuple('libzip', keyset['libzip/src/LICENSE']),
      'lz4' => tuple('LZ4', keyset['lz4/src/lib/LICENSE']),
      'mcrouter' => null, // first-party
      'ocaml' => null, // not distributed
      'opam' => null, // not used yet
      'pcre' => tuple('PCRE', keyset['forks/pcre/LICENCE']),
      'proxygen' => null, // first-party
      'ragel' => null, // not distributed
      're2' => tuple('RE2', keyset['re2/src/LICENSE']),
      'rsocket-cpp' => null, // first-party
      'rustc' => null, // not distributed
      'squangle' => null, // first-party
      'stubs' => null, // first-party
      'thrift' => null, // first-party
      'timelib' => tuple('Timelib', keyset['forks/timelib/LICENSE.rst']),
      'wangle' => null, // first-party
      'fb-mysql' => tuple(
        'MySQL Client',
        keyset[
          'fb-mysql/src/README', // GPL exception
          'fb-mysql/src/COPYING',
        ],
      ),
      'xed' => tuple('Intel XED Library', keyset['xed/xed/LICENSE']),
      'zstd' => null, // first-party
    ];
  }

  public static function printNotices(): void {
    print(
      "THE FOLLOWING SETS FORTH ATTRIBUTION NOTICES FOR THIRD PARTY SOFTWARE ".
      "THAT MAY BE CONTAINED IN PORTIONS OF THE FACEBOOK PRODUCT.\n"
    );
    $sentinel = 'DOES_NOT_EXIST';
    $known_projects = self::getKnownProjects();
    $projects = vec[];
    $dirs = (Vector {})
      ->addAll(new \DirectoryIterator(__DIR__.'/../forks/'))
      ->addAll(new \DirectoryIterator(__DIR__.'/../'));
    foreach ($dirs as $info) {
      if ($info->isDot() || !$info->isDir()) {
        continue;
      }
      $project = $info->getBasename();
      if (!\array_key_exists($project, $known_projects)) {
        \fprintf(\STDERR, "Failed to find config for project '%s'\n", $project);
        exit(1);
      }
      $projects[] = $project;
    }

    \sort(inout $projects);
    foreach ($projects as $project) {
      $config = $known_projects[$project] ?? null;
      if ($config === null) {
        continue;
      }
      list($name, $files) = $config;

      print("\n-----\n\n");
      \printf(
        "The following software may be included in this product: %s. This ".
        "Software contains the following license and notice below: \n\n",
        $name,
      );
      foreach ($files as $file) {
        $file = __DIR__.'/../'.$file;
        if (!\file_exists($file)) {
          \fprintf(\STDERR, "%s does not exist\n", $file);
          exit(2);
        }
        print(\file_get_contents($file));
      }
    }
  }
}

<<__EntryPoint>>
function print_notices_main(): void {
  ThirdPartyBinaryNotices::printNotices();
}
