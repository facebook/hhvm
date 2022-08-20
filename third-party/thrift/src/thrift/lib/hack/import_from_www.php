<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class ImportThriftLibFromWWW {
  public static function run(array<string> $argv): void {
    invariant(count($argv) >= 2, 'Need to specify path to www');
    self::importHackLib($argv[1]);
    self::transpileHack();
  }

  protected static function importHackLib(string $www_path): void {
    self::walkPHPFiles(
      $www_path.'/flib/thrift/core',
      array(''),
      function (string $root, string $path, string $file) {
        if (strpos($path, '__tests__') !== false) {
          echo "Skipping $path/$file\n";
          return;
        }

        $contents = file_get_contents($root.$path.'/'.$file);
        $contents = preg_replace(
          '#/\* BEGIN_STRIP \*/.*?/\* END_STRIP \*/#s',
          '',
          $contents,
        );
        $contents = preg_replace(
          '#/\* INSERT (.*?) END_INSERT \*/#s',
          '\\1',
          $contents,
        );

        $license = array(
          '/**',
          '* Copyright (c) 2006- Facebook',
          '* Distributed under the Thrift Software License',
          '*',
          '* See accompanying file LICENSE or visit the Thrift site at:',
          '* http://developers.facebook.com/thrift/',
          '*',
          '* @package '.str_replace('/', '.',dirname('thrift'.$path.'/'.$file)),
          '*/',
        );
        $replacement = "\\1\n".implode("\n", $license)."\n";
        $contents = preg_replace(
          '#^(<\?hh(?: +// +[a-z]+)?\n)//.*\n#',
          $replacement,
          $contents,
        );

        if (!file_exists(__DIR__.'/src/'.$path)) {
          mkdir(__DIR__.'/src/'.$path, 0755, true);
        }
        file_put_contents(__DIR__.'/src/'.$path.'/'.$file, $contents);
        echo "Imported $path/$file\n";
      }
    );
  }

  protected static function transpileHack(): void {
    $transpiler = realpath(__DIR__.'/../../../_bin/hphp/hack/src/h2tp/h2tp');
    $source = realpath(__DIR__.'/src');
    $dest = realpath(__DIR__.'/../php/src');

    system('rm -rf '.$dest);
    system($transpiler.' '.$source.' '.$dest);

    file_put_contents($dest.'/Thrift.php', implode("\n", array(
      '<?php',
      'if (!isset($GLOBALS[\'THRIFT_ROOT\'])) {',
      '  $GLOBALS[\'THRIFT_ROOT\'] = __DIR__;',
      '}',
      '',
      '// This file was split into several separate files',
      '// Now we can just require_once all of them',
      '',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TType.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TMessageType.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TException.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TBase.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/IThriftClient.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/IThriftProcessor.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/IThriftStruct.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TProcessorEventHandler.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TClientEventHandler.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/TApplicationException.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/protocol/TProtocol.php\';',
      'require_once $GLOBALS[\'THRIFT_ROOT\'].\'/transport/TTransport.php\';',
      '',
    )));

    file_put_contents($dest.'/autoload.php', implode("\n", array(
      '<?php',
      '// Thrift autoload support is deprecated.',
      '// Instead, use the Composer autoloader or another',
      '// autoloader implementation.',
      '',
    )));

    $nonclass_files = ImmSet {
      '/Thrift.php',
      '/autoload.php',
    };
    $classes = Map {};

    self::walkPHPFiles(
      $dest,
      array(''),
      function (string $root, string $path, string $file) use ($classes, $nonclass_files) {
        if (strpos($path, '__tests__') !== false) {
          echo "Skipping $path/$file\n";
          return;
        }

        if (!$nonclass_files->contains($path.'/'.$file)) {
          $classes[basename($file, '.php')] = $path.'/'.$file;
        }

        $contents = file_get_contents($root.$path.'/'.$file);

        $header = array(
          '<?php',
          '',
          '/**',
          '* Copyright (c) 2006- Facebook',
          '* Distributed under the Thrift Software License',
          '*',
          '* See accompanying file LICENSE or visit the Thrift site at:',
          '* http://developers.facebook.com/thrift/',
          '*',
          '* @package '.str_replace('/', '.', dirname('thrift'.$path.'/'.$file)),
          '*/',
          '',
          '',
        );

        $contents = preg_replace('#<\?php\n#', implode("\n", $header), $contents);

        file_put_contents($root.$path.'/'.$file, $contents);

        echo "Added header to $path/$file\n";
      }
    );

    self::walkPHPFiles(
      $dest,
      array(''),
      function (string $root, string $path, string $file) use ($classes, $nonclass_files) {
        if (strpos($path, '__tests__') !== false) {
          echo "Skipping $path/$file\n";
          return;
        }

        if ($nonclass_files->contains($path.'/'.$file)) {
          return;
        }

        $contents = file_get_contents($root.$path.'/'.$file);

        $includes = Vector {};
        $skip_includes = ImmMap {
          'TProtocol.php' => ImmSet {
            'TBinaryProtocolAccelerated',
            'TBinaryProtocolUnaccelerated',
            'TCompactProtocolAccelerated',
            'TCompactProtocolUnaccelerated',
          },
        };
        foreach ($classes as $class => $classpath) {
          if ($file === $class.'.php') {
            continue;
          }
          if ($skip_includes->containsKey($file)) {
            if ($skip_includes->at($file)->contains($class)) {
              continue;
            }
          }
          if (preg_match('/'.$class.'[^a-zA-Z]/', $contents)) {
            $includes[] = $classpath;
          }
        }

        if (!$includes->isEmpty()) {
          sort($includes);
          $hacklib = 'require_once ($GLOBALS["HACKLIB_ROOT"]);';
          $thrift_root_path = '__DIR__';

          $subdirs = substr_count($path, '/');
          if ($subdirs > 0) {
            $thrift_root_path = '__DIR__.\''.str_repeat('/..', $subdirs).'\'';
          }

          $thrift_root = implode("\n", array(
            'if (!isset($GLOBALS[\'THRIFT_ROOT\'])) {',
            '  $GLOBALS[\'THRIFT_ROOT\'] = '.$thrift_root_path.';',
            '}',
          ));

          $replacement = "$hacklib\n$thrift_root\n".implode("\n", $includes->map(function ($path): string {
            return 'require_once $GLOBALS[\'THRIFT_ROOT\'].\''.$path.'\';';
          }));

          $contents = str_replace(
            $hacklib,
            $replacement,
            $contents,
          );

          file_put_contents($root.$path.'/'.$file, $contents);

          echo "Added includes to $path/$file\n";
        }
      }
    );
  }

  protected static function walkPHPFiles(
    string $root,
    array<string> $paths,
    (function (string, string, string): void) $cb,
  ): void {
    for ($i = 0; $i < count($paths); $i++) {
      $path = $paths[$i];
      $full_path = $root.$path;

      $files = scandir($full_path);
      foreach ($files as $file) {
        if ($file === '.' || $file === '..') {
          continue;
        }

        if (is_dir($full_path.'/'.$file)) {
          $paths[] = $path.'/'.$file;
        }

        if (!preg_match('/\.php$/', $file)) {
          continue;
        }

        $cb($root, $path, $file);
      }
    }
  }
}

ImportThriftLibFromWWW::run($argv);
