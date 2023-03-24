---
title: NodeJS
category: Invocation
sidebar_position: 6
---

To install the nodejs client:

```bash
$ npm install fb-watchman
```

and to import it and create a client instance:

```js
var watchman = require('fb-watchman');
var client = new watchman.Client();
```

This documentation assumes that you are using the latest available version of
the `fb-watchman` package published to the npm repository.

## Checking for watchman availability

The client can be installed without requiring that the service is installed. It
is important to handle lack of availability and also to test whether the
installed service supports the [capabilities](/watchman/docs/capabilities.html)
required by your application.

The `capabilityCheck` method issues a
[version](/watchman/docs/cmds/version.html) command to query the capabilities of
the server.

```js
var watchman = require('fb-watchman');
var client = new watchman.Client();
client.capabilityCheck({optional:[], required:['relative_root']},
  function (error, resp) {
    if (error) {
      // error will be an Error object if the watchman service is not
      // installed, or if any of the names listed in the `required`
      // array are not supported by the server
      console.error(error);
    }
    // resp will be an extended version response:
    // {'version': '3.8.0', 'capabilities': {'relative_root': true}}
    console.log(resp);
  });
```

## Initiating a watch

Almost every operation in watchman revolves around watching a directory tree.
You can repeatedly ask to watch the same directory without error; watchman will
re-use an existing watch.

```js
var watchman = require('fb-watchman');
var client = new watchman.Client();

var dir_of_interest = "/some/path";

client.capabilityCheck({optional:[], required:['relative_root']},
  function (error, resp) {
    if (error) {
      console.log(error);
      client.end();
      return;
    }

    // Initiate the watch
    client.command(['watch-project', dir_of_interest],
      function (error, resp) {
        if (error) {
          console.error('Error initiating watch:', error);
          return;
        }

        // It is considered to be best practice to show any 'warning' or
        // 'error' information to the user, as it may suggest steps
        // for remediation
        if ('warning' in resp) {
          console.log('warning: ', resp.warning);
        }

        // `watch-project` can consolidate the watch for your
        // dir_of_interest with another watch at a higher level in the
        // tree, so it is very important to record the `relative_path`
        // returned in resp

        console.log('watch established on ', resp.watch,
                    ' relative_path', resp.relative_path);
      });
  });
```

## Subscribing to changes

Most node applications are interested in subscribing to live file change
notifications. In watchman these are configured by issuing a
[subscribe](/watchman/docs/cmd/subscribe.html) command. A subscription is valid
for the duration of your client connection, or until you cancel the subscription
using the [unsubscribe](/watchman/docs/cmd/unsubscribe.html) command.

The following will generate subscription results for all files in the tree that
match the query expression and then generate subscription results as files
change:

```js
// `watch` is obtained from `resp.watch` in the `watch-project` response.
// `relative_path` is obtained from `resp.relative_path` in the
// `watch-project` response.
function make_subscription(client, watch, relative_path) {
  sub = {
    // Match any `.js` file in the dir_of_interest
    expression: ["allof", ["match", "*.js"]],
    // Which fields we're interested in
    fields: ["name", "size", "mtime_ms", "exists", "type"]
  };
  if (relative_path) {
    sub.relative_root = relative_path;
  }

  client.command(['subscribe', watch, 'mysubscription', sub],
    function (error, resp) {
      if (error) {
        // Probably an error in the subscription criteria
        console.error('failed to subscribe: ', error);
        return;
      }
      console.log('subscription ' + resp.subscribe + ' established');
    });

  // Subscription results are emitted via the subscription event.
  // Note that this emits for all subscriptions.  If you have
  // subscriptions with different `fields` you will need to check
  // the subscription name and handle the differing data accordingly.
  // `resp`  looks like this in practice:
  //
  // { root: '/private/tmp/foo',
  //   subscription: 'mysubscription',
  //   files: [ { name: 'node_modules/fb-watchman/index.js',
  //       size: 4768,
  //       exists: true,
  //       type: 'f' } ] }
  client.on('subscription', function (resp) {
    if (resp.subscription !== 'mysubscription') return;

    resp.files.forEach(function (file) {
      // convert Int64 instance to javascript integer
      const mtime_ms = +file.mtime_ms;

      console.log('file changed: ' + file.name, mtime_ms);
    });
  });
}
```

### Subscribing only to changed files

The example above will generate results for existing (and deleted!) files at the
time that the subscription is established. In some applications this can be
undesirable. The following example shows how to add a logical time constraint.

watchman tracks changes using an
[abstract clock](/watchman/docs/clockspec.html). We'll determine the current
clock at the time that we initiate the watch and then add that as a constraint
in our subscription.

```js
function make_time_constrained_subscription(client, watch, relative_path) {
  client.command(['clock', watch], function (error, resp) {
    if (error) {
      console.error('Failed to query clock:', error);
      return;
    }

    sub = {
      // Match any `.js` file in the dir_of_interest
      expression: ["allof", ["match", "*.js"]],
      // Which fields we're interested in
      fields: ["name", "size", "exists", "type"],
      // add our time constraint
      since: resp.clock
    };

    if (relative_path) {
      sub.relative_root = relative_path;
    }

    client.command(['subscribe', watch, 'mysubscription', sub],
      function (error, resp) {
        // handle the result here
      });
  });
}
```

## NodeJS API Reference

## Methods

### client.capabilityCheck(options, done)

The `capabilityCheck` method issues a
[version](/watchman/docs/cmds/version.html) command to query the capabilities of
the server.

If the server doesn't support capabilities, `capabilityCheck` will emulate the
capability response for a handful of significant capabilities based on the
version reported by the server.

The `options` argument may contain the following properties:

- `optional` an array listing optional capability names
- `required` an array listing required capability names

The properties are passed through to the underlying `version` command.

The `done` parameter is a callback that will be passed (error, result) when the
command completes. It doesn't make sense to issue a `capabilityCheck` call and
not provide the `done` callback.

The response object will contain a `capabilities` object property whose keys
will be the union of the `optional` and `required` capability names and whose
values will be either `true` or `false` depending on the availability of the
capability name.

If any of the `required` capabilities are not supported by the server, the
`error` parameter in the `done` callback will be set and will contain a
meaningful error message.

```js
client.capabilityCheck({optional:[], required:['relative_root']},
  function (error, resp) {
    if (error) {
      // error will be an Error object if the watchman service is not
      // installed, or if any of the names listed in the `required`
      // array are not supported by the server
      console.error(error);
    }
    // resp will be an extended version response:
    // {'version': '3.8.0', 'capabilities': {'relative_root': true}}
    console.log(resp);
  });
```

### client.command(args [, done])

Sends a command to the watchman service. `args` is an array that specifies the
command name and any optional arguments. The command is queued and dispatched
asynchronously. You may queue multiple commands to the service; they will be
dispatched in FIFO order once the client connection is established.

The `done` parameter is a callback that will be passed (error, result) when the
command completes. You may omit it if you are not interested in the result of
the command.

```js
client.command(['watch-project', process.cwd()], function(error, resp) {
  if (error) {
    console.log('watch failed: ', error);
    return;
  }
  if ('warning' in resp) {
    console.log('warning: ', resp.warning);
  }
  if ('relative_path' in resp) {
    // We will need to remember and adjust for relative_path
    console.log('watching project ', resp.watch, ' relative path to cwd is ',
      resp.relative_path);
  } else {
    console.log('watching ', resp.watch);
  }
});
```

If a field named `warning` is present in `resp`, the watchman service is trying
to communicate an issue that the user should see and address. For example, if
the system watch resources need adjustment, watchman will provide information
about this and how to remediate the issue. It is suggested that tools that build
on top of this library bubble the warning message up to the user.

### client.end()

Terminates the connection to the watchman service. Does not wait for any queued
commands to send.

## Events

The following events are emitted by the watchman client object:

### Event: 'connect'

Emitted when the client successfully connects to the watchman service

### Event: 'error'

Emitted when the socket to the watchman service encounters an error.

It may also be emitted prior to establishing a connection if we are unable to
successfully execute the watchman CLI binary to determine how to talk to the
server process.

It is passed a variable that encapsulates the error.

### Event: 'end'

Emitted when the socket to the watchman service is closed

### Event: 'log'

Emitted in response to a unilateral `log` PDU from the watchman service. To
enable these, you need to send a `log-level` command to the service:

```js
// This is very verbose, you probably don't want to do this
client.command(['log-level', 'debug']);
client.on('log', function(info) {
  console.log(info);
});
```

### Event: 'subscription'

Emitted in response to a unilateral `subscription` PDU from the watchman
service. To enable these, you need to send a `subscribe` command to the service:

```js
  // Subscribe to notifications about .js files
  client.command(['subscribe', process.cwd(), 'mysubscription', {
      expression: ["match", "*.js"]
    }],
    function(error, resp) {
      if (error) {
        // Probably an error in the subscription criteria
        console.log('failed to subscribe: ', error);
        return;
      }
      console.log('subscription ' + resp.subscribe + ' established');
    }
  );

  // Subscription results are emitted via the subscription event.
  // Note that watchman will deliver a list of all current files
  // when you first subscribe, so you don't need to walk the tree
  // for yourself on startup
  client.on('subscription', function(resp) {
    console.log(resp.root, resp.subscription, resp.files);
  });
```

To cancel a subscription, use the `unsubscribe` command and pass in the name of
the subscription you want to cancel:

```js
  client.command(['unsubscribe', process.cwd(), 'mysubscription']);
```

Note that subscriptions names are scoped to your connection to the watchman
service; multiple different clients can use the same subscription name without
fear of colliding.
