<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\Vec;
use namespace HH\Lib\{IO, Network, OS, Str, TCP};

use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;
use type HH\__Private\MiniTest\DataProvider;
use type HH\Lib\Network\{IPProtocolBehavior, IPProtocolVersion};
use type HH\Lib\Ref;

final class HSLTCPTest extends HackTest {
  public static function provideConnectionParameters(
  ): vec<(IPProtocolVersion, string, string, IPProtocolBehavior)> {
    return vec[
      tuple(
        IPProtocolVersion::IPV6,
        'localhost',
        '::1',
        IPProtocolBehavior::PREFER_IPV6,
      ),
      tuple(
        IPProtocolVersion::IPV6,
        'localhost',
        'localhost',
        IPProtocolBehavior::PREFER_IPV6,
      ),
      tuple(
        IPProtocolVersion::IPV6,
        'localhost',
        '::1',
        IPProtocolBehavior::FORCE_IPV6,
      ),
      tuple(
        IPProtocolVersion::IPV4,
        'localhost',
        '127.0.0.1',
        IPProtocolBehavior::PREFER_IPV6,
      ),
      tuple(
        IPProtocolVersion::IPV4,
        'localhost',
        'localhost',
        IPProtocolBehavior::PREFER_IPV6,
      ),
      tuple(
        IPProtocolVersion::IPV4,
        'localhost',
        '127.0.0.1',
        IPProtocolBehavior::FORCE_IPV4,
      ),
    ];
  }

  <<DataProvider('provideConnectionParameters')>>
  public async function testBasicConnectivity(
    IPProtocolVersion $server_protocol,
    string $bind_address,
    string $client_address,
    IPProtocolBehavior $client_protocol,
  ): Awaitable<void> {
    try {
      $server = await TCP\Server::createAsync($server_protocol, $bind_address, 0);
    } catch (OS\ErrnoException $e) {
      expect($e->getErrno())->toEqual(
        OS\Errno::EADDRNOTAVAIL,
        'Expected EADDRNOTAVAIL, got %s',
        $e->getMessage(),
      );
      expect($server_protocol)->toEqual(IPProtocolVersion::IPV6);
      self::markTestSkipped("IPv6 not supported on this host");
      return;
    }
    list($host, $port) = $server->getLocalAddress();
    expect($host)->toNotEqual($bind_address);
    expect($port)->toNotEqual(0);
    $server_recv = new Ref('');
    $client_recv = new Ref('');
    concurrent {
      await async {
        ///// Server /////
        $client = await $server->nextConnectionAsync();
        $server_recv->value = await $client->readAllowPartialSuccessAsync();
        await $client->writeAllowPartialSuccessAsync("foo\n");
        $client->close();
      };
      await async {
        ///// client /////
        try {
          $conn = await TCP\connect_async(
            $client_address,
            $port,
            shape('ip_version' => $client_protocol),
          );
        } catch (OS\ErrnoException $e) {
          throw $e;
        }
        list($ph, $pp) = $conn->getPeerAddress();
        $expected = vec[$host];
        if (
          $host === '127.0.0.1' &&
          $client_protocol === IPProtocolBehavior::PREFER_IPV6
        ) {
          $expected[] = '::ffff:'.$host;
        }
        expect($expected)->toContain($host);
        expect($pp)->toEqual($port);
        list($lh, $lp) = $conn->getLocalAddress();
        expect($lh)->toEqual($ph);
        expect($lp)->toNotEqual($pp);
        await $conn->writeAllowPartialSuccessAsync("bar\n");
        $client_recv->value = await $conn->readAllowPartialSuccessAsync();
        $conn->close();
      };
    }
    expect($client_recv->value)->toEqual("foo\n");
    expect($server_recv->value)->toEqual("bar\n");
  }

  public async function testConnectingToInvalidPort(): Awaitable<void> {
    $ex = expect(async () ==> await TCP\connect_async('localhost', 0))
      ->toThrow(OS\ErrnoException::class);
    expect(vec[OS\Errno::EADDRNOTAVAIL, OS\Errno::ECONNREFUSED])
      ->toContain($ex->getErrno());
  }

  public async function testReuseAddress(): Awaitable<void> {
    $s1 = await TCP\Server::createAsync(
      IPProtocolVersion::IPV4,
      '127.0.0.1',
      0,
      // Portability:
      // - MacOS only requires SO_REUSEADDR to be set on the new socket
      // - Linux requires SO_REUSEADDR on both the old and the new socket
      shape('socket_options' => shape('SO_REUSEADDR' => true)),
    );
    $port = $s1->getLocalAddress()[1];
    concurrent {
      $client = await TCP\connect_async('127.0.0.1', $port);
      $_ = await $s1->nextConnectionAsync();
    }
    await $client->writeAllowPartialSuccessAsync('hello, world');
    $s1->stopListening();

    $ex = expect(
      async () ==> await TCP\Server::createAsync(
        IPProtocolVersion::IPV4,
        '127.0.0.1',
        $port,
      ),
    )->toThrow(OS\ErrnoException::class);
    expect($ex->getErrno())->toEqual(OS\Errno::EADDRINUSE);

    $s2 = await TCP\Server::createAsync(
      IPProtocolVersion::IPV4,
      '127.0.0.1',
      $port,
      shape('socket_options' => shape('SO_REUSEADDR' => true)),
    );
    concurrent {
      $client_recv = await async {
        $client = await TCP\connect_async('127.0.0.1', $port);
        await $client->writeAllowPartialSuccessAsync("hello, world!\n");
        $result = await $client->readAllowPartialSuccessAsync();
        $client->close();
        return $result;
      };
      $server_recv = await async {
        $conn = await $s2->nextConnectionAsync();
        await $conn->writeAllowPartialSuccessAsync("foo bar\n");
        $result = await $conn->readAllowPartialSuccessAsync();
        $conn->close();
        return $result;
      };
    }
    expect($client_recv)->toEqual("foo bar\n");
    expect($server_recv)->toEqual("hello, world!\n");
  }

  public async function testCloseDuringAsyncAccept(): Awaitable<void> {
    $s = await TCP\Server::createAsync(
      IPProtocolVersion::IPV4,
      '127.0.0.1',
      0,
    );
    // intentionally not awaiting
    $accept_awaitable = $s->nextConnectionAsync();
    $s->stopListening();
    $ex = expect(async () ==> await $accept_awaitable)->toThrow(
      OS\ErrnoException::class,
      'Server socket closed while waiting for connection',
    );
    expect($ex->getErrno())->toEqual(OS\Errno::ECONNABORTED);
  }
}
