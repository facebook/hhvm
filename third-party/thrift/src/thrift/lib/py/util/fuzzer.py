# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

"""
Fuzz Testing for Thrift Services
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import collections
import inspect
import json
import logging
import os
import pprint
import sys
import time

import six

# pyre-fixme[21]: Could not find module `six.moves`.
import six.moves as sm

# pyre-fixme[21]: Could not find module `six.moves.urllib.parse`.
from six.moves.urllib.parse import urlparse

try:
    # pyre-fixme[21]: Could not find module `ServiceRouter`.
    from ServiceRouter import ConnConfigs, ServiceOptions, ServiceRouter  # @manual

    SR_AVAILABLE = True
except ImportError:
    SR_AVAILABLE = False

from thrift import Thrift
from thrift.protocol import TBinaryProtocol, TCompactProtocol, THeaderProtocol
from thrift.transport import THttpClient, TSocket, TSSLSocket, TTransport
from thrift.util import randomizer

if six.PY3:
    from importlib.machinery import SourceFileLoader

    def load_source(name, pathname):
        return SourceFileLoader(name, pathname).load_module()

else:
    import imp

    def load_source(name, pathname):
        return imp.load_source(name, pathname)


def positive_int(s) -> int:
    """Typechecker for positive integers"""
    try:
        n = int(s)
        if not n > 0:
            raise argparse.ArgumentTypeError("%s is not positive." % s)
        return n
    except ValueError:
        raise argparse.ArgumentTypeError("Cannot convert %s to an integer." % s)


def prob_float(s) -> float:
    """Typechecker for probability values"""
    try:
        x = float(s)
        if not 0 <= x <= 1:
            raise argparse.ArgumentTypeError("%s is not a valid probability." % x)
        return x
    except ValueError:
        raise argparse.ArgumentTypeError("Cannot convert %s to a float." % s)


class FuzzerConfiguration(object):
    """Container for Fuzzer configuration options"""

    argspec = {
        "allow_application_exceptions": {
            "description": "Do not flag TApplicationExceptions as errors",
            "type": bool,
            "flag": "-a",
            "argparse_kwargs": {"action": "store_const", "const": True},
            "default": False,
        },
        "compact": {
            "description": "Use TCompactProtocol",
            "type": bool,
            "flag": "-c",
            "argparse_kwargs": {"action": "store_const", "const": True},
            "default": False,
        },
        "constraints": {
            "description": "JSON Constraint dictionary",
            "type": str,
            "flag": "-Con",
            "default": {},
            "is_json": True,
        },
        "framed": {
            "description": "Use framed transport.",
            "type": bool,
            "flag": "-f",
            "argparse_kwargs": {"action": "store_const", "const": True},
            "default": False,
        },
        "functions": {
            "description": "Which functions to test. If excluded, test all",
            "type": str,
            "flag": "-F",
            "argparse_kwargs": {
                "nargs": "*",
            },
            "default": None,
        },
        "host": {
            "description": "The host and port to connect to",
            "type": str,
            "flag": "-h",
            "argparse_kwargs": {"metavar": "HOST[:PORT]"},
            "default": None,
        },
        "iterations": {
            "description": "Number of calls per method.",
            "type": positive_int,
            "flag": "-n",
            "attr_name": "n_iterations",
            "default": 1000,
        },
        "logfile": {
            "description": "File to write output logs.",
            "type": str,
            "flag": "-l",
            "default": None,
        },
        "loglevel": {
            "description": "Level of verbosity to write logs.",
            "type": str,
            "flag": "-L",
            "argparse_kwargs": {
                "choices": ["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"],
            },
            "default": "INFO",
        },
        "service": {
            "description": "Path to file of Python service module.",
            "type": str,
            "flag": "-S",
            "attr_name": "service_path",
            "default": None,
        },
        "ssl": {
            "description": "Use SSL socket.",
            "type": bool,
            "flag": "-s",
            "argparse_kwargs": {"action": "store_const", "const": True},
            "default": False,
        },
        "unframed": {
            "description": "Use unframed transport.",
            "type": bool,
            "flag": "-U",
            "argparse_kwargs": {"action": "store_const", "const": True},
            "default": False,
        },
        "url": {
            "description": "The URL to connect to for HTTP transport",
            "type": str,
            "flag": "-u",
            "default": None,
        },
    }
    if SR_AVAILABLE:
        argspec["tier"] = {
            "description": "The SMC tier to connect to",
            "type": str,
            "flag": "-t",
            "default": None,
        }
        argspec["conn_configs"] = {
            "description": "ConnConfigs to use for ServiceRouter connection",
            "type": str,
            "flag": "-Conn",
            "default": {},
            "is_json": True,
        }
        argspec["service_options"] = {
            "description": "ServiceOptions to use for ServiceRouter connection",
            "type": str,
            "flag": "-SO",
            "default": {},
            "is_json": True,
        }

    def __init__(self, service=None):
        cls = self.__class__

        if service is not None:
            self.service = service

        parser = argparse.ArgumentParser(
            description="Fuzzer Configuration", add_help=False
        )
        parser.add_argument(
            "-C",
            "--config",
            dest="config_filename",
            help="JSON Configuration file. "
            "All settings can be specified as commandline "
            "args and config file settings. Commandline args "
            "override config file settings.",
        )

        parser.add_argument(
            "-?", "--help", action="help", help="Show this help message and exit."
        )

        for name, arg in six.iteritems(cls.argspec):
            kwargs = arg.get("argparse_kwargs", {})

            if kwargs.get("action", None) != "store_const":
                # Pass type to argparse. With store_const, type can be inferred
                kwargs["type"] = arg["type"]

            # If an argument is not passed, don't put a value in the namespace
            kwargs["default"] = argparse.SUPPRESS

            # Use the argument's description and default as a help message
            kwargs["help"] = "%s Default: %s" % (
                arg.get("description", ""),
                arg["default"],
            )

            kwargs["dest"] = arg.get("attr_name", name)

            if hasattr(self, kwargs["dest"]):
                # Attribute already assigned (e.g., service passed to __init__)
                continue

            parser.add_argument(arg["flag"], "--%s" % name, **kwargs)

            # Assign the default value to config namespace
            setattr(self, kwargs["dest"], arg["default"])

        args = parser.parse_args()

        # Read settings in config file
        self.__dict__.update(cls._config_file_settings(args))

        # Read settings in args
        self.__dict__.update(cls._args_settings(args))

        valid, message = self._validate_config()
        if not valid:
            print(message, file=sys.stderr)
            sys.exit(os.EX_USAGE)

    @classmethod
    def _try_parse_type(cls, name, type_, val):
        try:
            val = type_(val)
        except ValueError:
            raise TypeError(
                ("Expected type %s for setting %s, " "but got type %s (%s)")
                % (type_, name, type(val), val)
            )
        return val

    @classmethod
    def _try_parse(cls, name, arg, val):
        if arg.get("is_json", False):
            return val

        type_ = arg["type"]

        nargs = arg.get("argparse_kwargs", {}).get("nargs", None)

        if nargs is None:
            return cls._try_parse_type(name, type_, val)
        else:
            if not isinstance(val, list):
                raise TypeError(
                    (
                        "Expected list of length %s "
                        "for setting %s, but got type %s (%s)"
                    )
                    % (nargs, name, type(val), val)
                )
            ret = []
            for elem in val:
                ret.append(cls._try_parse_type(name, type_, elem))
            return ret

    @classmethod
    def _config_file_settings(cls, args):
        """Read settings from a configuration file"""
        if args.config_filename is None:
            return {}  # No config file
        if not os.path.exists(args.config_filename):
            raise OSError(
                os.EX_NOINPUT, "Config file does not exist: %s" % args.config_filename
            )
        with open(args.config_filename, "r") as fd:
            try:
                settings = json.load(fd)
            except ValueError as e:
                raise ValueError("Error parsing config file: %s" % e)

        # Make sure settings are well-formatted
        renamed_settings = {}
        if not isinstance(settings, dict):
            raise TypeError("Invalid config file. Top-level must be Object.")
        for name, val in six.iteritems(settings):
            if name not in cls.argspec:
                raise ValueError(("Unrecognized configuration " "option: %s") % name)
            arg = cls.argspec[name]
            val = cls._try_parse(name, arg, val)
            attr_name = arg.get("attr_name", name)
            renamed_settings[attr_name] = val
        return renamed_settings

    @classmethod
    def _args_settings(cls, args):
        """Read settings from the args namespace returned by argparse"""
        settings = {}
        for name, arg in six.iteritems(cls.argspec):
            attr_name = arg.get("attr_name", name)
            if not hasattr(args, attr_name):
                continue
            value = getattr(args, attr_name)
            if arg.get("is_json", False):
                settings[attr_name] = json.loads(value)
            else:
                settings[attr_name] = value
        return settings

    def __str__(self):
        return "Configuration(\n%s\n)" % pprint.pformat(self.__dict__)

    def load_service(self):
        if self.service is not None:
            if self.service_path is not None:
                raise ValueError(
                    "Cannot specify a service path when the "
                    "service is input programmatically"
                )
            # Service already loaded programmatically. Just load methods.
            self.service.load_methods()
            return self.service

        if self.service_path is None:
            raise ValueError("Error: No service specified")

        service_path = self.service_path

        if not os.path.exists(service_path):
            raise OSError("Service module does not exist: %s" % service_path)

        if not service_path.endswith(".py"):
            raise OSError("Service module is not a Python module: %s" % service_path)

        parent_path, service_filename = os.path.split(service_path)
        service_name = service_filename[:-3]  # Truncate extension

        logging.info("Service name: %s" % (service_name))

        parent_path = os.path.dirname(service_path)
        ttypes_path = os.path.join(parent_path, "ttypes.py")
        constants_path = os.path.join(parent_path, "constants.py")

        load_source("module", parent_path)
        ttypes_module = load_source("module.ttypes", ttypes_path)
        constants_module = load_source("module.constants", constants_path)
        service_module = load_source("module.%s" % (service_name), service_path)

        service = Service(ttypes_module, constants_module, service_module)
        service.load_methods()
        return service

    def _validate_config(self):
        # Verify there is one valid connection flag
        specified_flags = []
        connection_flags = FuzzerClient.connection_flags
        for flag in connection_flags:
            if hasattr(self, flag) and getattr(self, flag) is not None:
                specified_flags.append(flag)

        if not len(specified_flags) == 1:
            message = "Exactly one of [%s] must be specified. Got [%s]." % (
                (", ".join("--%s" % flag for flag in connection_flags)),
                (", ".join("--%s" % flag for flag in specified_flags)),
            )
            return False, message

        connection_method = specified_flags[0]
        self.connection_method = connection_method

        if connection_method == "url":
            if not (self.compact or self.framed or self.unframed):
                message = (
                    "A protocol (compact, framed, or unframed) "
                    "must be specified for HTTP Transport."
                )
                return False, message

        if connection_method in {"url", "host"}:
            if connection_method == "url":
                try:
                    url = urlparse(self.url)
                except Exception:
                    return False, "Unable to parse url %s" % self.url
                else:
                    connection_str = url[1]
            elif connection_method == "host":
                connection_str = self.host
            if ":" in connection_str:
                # Get the string after the colon
                port = connection_str[connection_str.index(":") + 1 :]
                try:
                    int(port)
                except ValueError:
                    message = "Port is not an integer: %s" % port
                    return False, message

        return True, None


class Service(object):
    """Wrapper for a thrift service"""

    def __init__(self, ttypes_module, constants_module, service_module):
        self.ttypes = ttypes_module
        self.constants = constants_module
        self.service = service_module
        self.methods = None

    def __str__(self):
        return "Service(%s)" % self.service.__name__

    def load_methods(self, exclude_ifaces=None):
        """Load a service's methods.

        If exclude_ifaces is not None, it should be a collection and only
        methods from thrift interfaces not included in that collection will
        be considered."""

        exclude_ifaces = exclude_ifaces or []

        pred = inspect.isfunction if six.PY3 else inspect.ismethod

        methods = {}
        exclude_methods = []

        for klass in exclude_ifaces:
            exclude_methods.extend(inspect.getmembers(klass, predicate=pred))

        klass_methods = inspect.getmembers(self.service.Iface, predicate=pred)

        for method_name, method in klass_methods:
            if (method_name, method) in exclude_methods:
                continue

            module = inspect.getmodule(method)

            args = getattr(module, method_name + "_args", None)
            if args is None:
                continue
            result = getattr(module, method_name + "_result", None)

            thrift_exceptions = []
            if result is not None:
                for res_spec in result.thrift_spec:
                    if res_spec is None:
                        continue
                    if res_spec[2] != "success":
                        # This is an exception return type
                        spec_args = res_spec[3]
                        exception_type = spec_args[0]
                        thrift_exceptions.append(exception_type)

            methods[method_name] = {
                "args_class": args,
                "result_spec": result,
                "thrift_exceptions": tuple(thrift_exceptions),
            }

        self.methods = methods

    @property
    def client_class(self):
        return self.service.Client

    def get_methods(self, include=None):
        """Get a dictionary of methods provided by the service.

        If include is not None, it should be a collection and only
        the method names in that collection will be included."""

        if self.methods is None:
            raise ValueError(
                "Service.load_methods must be " "called before Service.get_methods"
            )

        if include is None:
            return self.methods

        included_methods = {}
        for method_name in include:
            if method_name not in self.methods:
                raise NameError("Function does not exist: %s" % method_name)
            included_methods[method_name] = self.methods[method_name]

        return included_methods


class FuzzerClient(object):
    """Client wrapper used to make calls based on configuration settings"""

    connection_flags = ["host", "url", "tier"]
    default_port = 9090

    def __init__(self, config, client_class):
        self.config = config
        self.client_class = client_class

    def _get_client_by_transport(self, config, transport, socket=None):
        # Create the protocol and client
        if config.compact:
            protocol = TCompactProtocol.TCompactProtocol(transport)
        # No explicit option about protocol is specified. Try to infer.
        elif config.framed or config.unframed:
            protocol = TBinaryProtocol.TBinaryProtocolAccelerated(transport)
        elif socket is not None:
            protocol = THeaderProtocol.THeaderProtocol(socket)
            transport = protocol.trans
        else:
            raise ValueError("No protocol specified for HTTP Transport")
        transport.open()
        self._transport = transport

        client = self.client_class(protocol)
        return client

    def _parse_host_port(self, value, default_port):
        parts = value.rsplit(":", 1)
        if len(parts) == 1:
            return (parts[0], default_port)
        else:
            # FuzzerConfiguration ensures parts[1] is an int
            return (parts[0], int(parts[1]))

    def _get_client_by_host(self):
        config = self.config
        host, port = self._parse_host_port(config.host, self.default_port)
        socket = (
            TSSLSocket.TSSLSocket(host, port)
            if config.ssl
            else TSocket.TSocket(host, port)
        )
        if config.framed:
            transport = TTransport.TFramedTransport(socket)
        else:
            transport = TTransport.TBufferedTransport(socket)
        return self._get_client_by_transport(config, transport, socket=socket)

    def _get_client_by_url(self):
        config = self.config
        url = urlparse(config.url)
        host, port = self._parse_host_port(url[1], 80)
        transport = THttpClient.THttpClient(config.url)
        return self._get_client_by_transport(config, transport)

    def _get_client_by_tier(self):
        """Get a client that uses ServiceRouter"""
        config = self.config
        serviceRouter = ServiceRouter()

        overrides = ConnConfigs()
        for key, val in six.iteritems(config.conn_configs):
            key = six.binary_type(key)
            val = six.binary_type(val)
            overrides[key] = val

        sr_options = ServiceOptions()
        for key, val in six.iteritems(config.service_options):
            key = six.binary_type(key)
            if not isinstance(val, list):
                raise TypeError(
                    "Service option %s expected list; got %s (%s)"
                    % (key, val, type(val))
                )
            val = [six.binary_type(elem) for elem in val]
            sr_options[key] = val

        service_name = config.tier

        # Obtain a normal client connection using SR2
        client = serviceRouter.getClient2(
            self.client_class, service_name, sr_options, overrides
        )

        if client is None:
            raise NameError("Failed to lookup host for tier %s" % service_name)

        return client

    def _get_client(self):
        if self.config.connection_method == "host":
            client = self._get_client_by_host()
        elif self.config.connection_method == "url":
            client = self._get_client_by_url()
        elif self.config.connection_method == "tier":
            client = self._get_client_by_tier()
        else:
            raise NameError(
                "Unknown connection type: %s" % self.config.connection_method
            )
        return client

    def _close_client(self):
        if self.config.connection_method in {"host", "url"}:
            self._transport.close()

    def __enter__(self):
        self.client = self._get_client()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._close_client()
        self.client = None

    def reset(self):
        self._close_client()
        try:
            self.client = self._get_client()
            return True
        except TTransport.TTransportException as e:
            logging.error("Unable to reset connection: %r" % e)
            return False

    def make_call(self, method_name, kwargs, is_oneway=False):
        method = getattr(self.client, method_name)
        ret = method(**kwargs)

        if is_oneway:
            self.reset()

        return ret


class Timer(object):
    def __init__(self, aggregator, category, action):
        self.aggregator = aggregator
        self.category = category
        self.action = action

    def __enter__(self):
        self.start_time = time.time()

    def __exit__(self, exc_type, exc_value, traceback):
        end_time = time.time()
        time_elapsed = end_time - self.start_time
        self.aggregator.add(self.category, self.action, time_elapsed)


class TimeAggregator(object):
    def __init__(self):
        self.total_time = collections.defaultdict(
            lambda: collections.defaultdict(float)
        )

    def time(self, category, action):
        return Timer(self, category, action)

    def add(self, category, action, time_elapsed):
        self.total_time[category][action] += time_elapsed

    def summarize(self):
        max_category_name_length = max(len(name) for name in self.total_time)
        max_action_name_length = max(
            max(len(action_name) for action_name in self.total_time[name])
            for name in self.total_time
        )
        category_format = "%%%ds: %%s" % max_category_name_length
        action_format = "%%%ds: %%4.3fs" % max_action_name_length

        category_summaries = []
        for category_name, category_actions in sorted(self.total_time.items()):
            timing_items = []
            for action_name, action_time in sorted(category_actions.items()):
                timing_items.append(action_format % (action_name, action_time))
            all_actions = " | ".join(timing_items)
            category_summaries.append(category_format % (category_name, all_actions))
        summaries = "\n".join(category_summaries)
        logging.info("Timing Summary:\n%s" % summaries)


class FuzzTester(object):
    summary_interval = 1  # Seconds between summary logs

    class Result:
        Success = 0
        TransportException = 1
        ApplicationException = 2
        UserDefinedException = 3
        OtherException = 4
        Crash = 5

    def __init__(self, config):
        self.config = config
        self.service = None
        self.randomizer = None
        self.client = None

    def start_logging(self):
        logfile = self.config.logfile
        if self.config.logfile is None:
            logfile = "/dev/null"
        log_level = getattr(logging, self.config.loglevel)

        datefmt = "%Y-%m-%d %H:%M:%S"
        fmt = "[%(asctime)s] [%(levelname)s] %(message)s"

        if logfile == "stdout":
            logging.basicConfig(stream=sys.stdout, level=log_level)
        else:
            logging.basicConfig(filename=self.config.logfile, level=log_level)

        log_handler = logging.getLogger().handlers[0]
        log_handler.setFormatter(logging.Formatter(fmt, datefmt=datefmt))

    def start_timing(self):
        self.timer = TimeAggregator()
        self.next_summary_time = time.time() + self.__class__.summary_interval

    def _call_string(self, method_name, kwargs):
        kwarg_str = ", ".join("%s=%s" % (k, v) for k, v in six.iteritems(kwargs))
        return "%s(%s)" % (method_name, kwarg_str)

    def run_test(
        self, method_name, kwargs, expected_output, is_oneway, thrift_exceptions
    ):
        """
        Make an RPC with given arguments and check for exceptions.
        """
        try:
            with self.timer.time(method_name, "Thrift"):
                self.client.make_call(method_name, kwargs, is_oneway)
        except thrift_exceptions as e:
            self.record_result(method_name, FuzzTester.Result.UserDefinedException)
            if self.config.loglevel == "DEBUG":
                with self.timer.time(method_name, "Logging"):
                    logging.debug("Got thrift exception: %r" % e)
                    logging.debug(
                        "Exception thrown by call: %s"
                        % (self._call_string(method_name, kwargs))
                    )

        except Thrift.TApplicationException as e:
            self.record_result(method_name, FuzzTester.Result.ApplicationException)
            if self.config.allow_application_exceptions:
                if self.config.loglevel == "DEBUG":
                    with self.timer.time(method_name, "Logging"):
                        logging.debug("Got TApplication exception %s" % e)
                        logging.debug(
                            "Exception thrown by call: %s"
                            % (self._call_string(method_name, kwargs))
                        )
            else:
                with self.timer.time(method_name, "Logging"):
                    self.n_exceptions += 1
                    logging.error("Got application exception: %s" % e)
                    logging.error(
                        "Offending call: %s" % (self._call_string(method_name, kwargs))
                    )

        except TTransport.TTransportException as e:
            self.n_exceptions += 1

            with self.timer.time(method_name, "Logging"):
                logging.error("Got TTransportException: (%s, %r)" % (e, e))
                logging.error(
                    "Offending call: %s" % (self._call_string(method_name, kwargs))
                )

            if "errno = 111: Connection refused" in e.args[0]:
                # Unable to connect to server - server may be down
                self.record_result(method_name, FuzzTester.Result.Crash)
                return False

            if not self.client.reset():
                logging.error("Inferring server crash.")
                self.record_result(method_name, FuzzTester.Result.Crash)
                return False

            self.record_result(method_name, FuzzTester.Result.TransportException)

        except Exception as e:
            self.record_result(method_name, FuzzTester.Result.OtherException)
            with self.timer.time(method_name, "Logging"):
                self.n_exceptions += 1
                logging.error("Got exception %s (%r)" % (e, e))
                logging.error(
                    "Offending call: %s" % (self._call_string(method_name, kwargs))
                )
                if hasattr(self, "previous_kwargs"):
                    logging.error(
                        "Previous call: %s"
                        % (self._call_string(method_name, self.previous_kwargs))
                    )

        else:
            self.record_result(method_name, FuzzTester.Result.Success)
            if self.config.loglevel == "DEBUG":
                with self.timer.time(method_name, "Logging"):
                    logging.debug(
                        "Successful call: %s" % (self._call_string(method_name, kwargs))
                    )
        finally:
            self.n_tests += 1

        return True

    def fuzz_kwargs(self, method_name, n_iterations):
        # For now, just yield n random sets of args
        # In future versions, fuzz fields more methodically based
        # on feedback and seeds
        for _ in sm.xrange(n_iterations):
            with self.timer.time(method_name, "Randomizing"):
                method_randomizer = self.method_randomizers[method_name]
                args_struct = method_randomizer.generate()
            if args_struct is None:
                logging.error("Unable to produce valid arguments for %s" % method_name)
            else:
                kwargs = args_struct.__dict__  # Get members of args struct
                yield kwargs

    def get_method_randomizers(self, methods, constraints):
        """Create a StructRandomizer for each method"""
        state = randomizer.RandomizerState()
        method_randomizers = {}

        state.push_type_constraints(constraints)

        for method_name in methods:
            method_constraints = constraints.get(method_name, {})
            args_class = methods[method_name]["args_class"]

            # Create a spec_args tuple for the method args struct type
            randomizer_spec_args = (
                args_class,
                args_class.thrift_spec,
                False,  # isUnion
            )

            method_randomizer = state.get_randomizer(
                Thrift.TType.STRUCT, randomizer_spec_args, method_constraints
            )
            method_randomizers[method_name] = method_randomizer

        return method_randomizers

    def _split_key(self, key):
        """Split a constraint rule key such as a.b|c into ['a', 'b', '|c']
        Dots separate hierarchical field names and property names

        Pipes indicate a type name and hashes indicate a field name,
        though these rules are not yet supported.
        """
        components = []
        start_idx = 0
        cur_idx = 0
        while cur_idx < len(key):
            if cur_idx != start_idx and key[cur_idx] in {".", "|", "#"}:
                components.append(key[start_idx:cur_idx])
                start_idx = cur_idx
                if key[cur_idx] == ".":
                    start_idx += 1
                cur_idx = start_idx
            else:
                cur_idx += 1
        components.append(key[start_idx:])
        return components

    def preprocess_constraints(self, source_constraints):
        """
        The constraints dictionary can have any key
        that follows the following format:

        method_name[.arg_name][.field_name ...].property_name

        The values in the dictionary can be nested such that inner field
        names are subfields of the outer scope, and inner type rules are
        applied only to subvalues of the out scope.

        After preprocessing, each dictionary level should have exactly one
        method name, field name, or property name as its key.

        Any strings of identifiers are converted into the nested dictionary
        structure. For example, the constraint set:

        {'my_method.my_field.distribution': 'uniform(0,100)'}

        Will be preprocessed to:

        {'my_method':
          {'my_field':
             {'distribution': 'uniform(0, 100)'}
          }
        }
        """
        constraints = {}
        scope_path = []

        def add_constraint(rule):
            walk_scope = constraints
            for key in scope_path[:-1]:
                if key not in walk_scope:
                    walk_scope[key] = {}
                walk_scope = walk_scope[key]
            walk_scope[scope_path[-1]] = rule

        def add_constraints_from_dict(d):
            for key, rule in six.iteritems(d):
                key_components = self._split_key(key)
                scope_path.extend(key_components)
                if isinstance(rule, dict):
                    add_constraints_from_dict(rule)
                else:
                    add_constraint(rule)
                scope_path[-len(key_components) :] = []

        add_constraints_from_dict(source_constraints)
        return constraints

    def start_result_counters(self):
        """Create result counters. The counters object is a dict that maps
        a method name to a counter of FuzzTest.Results
        """
        self.result_counters = collections.defaultdict(collections.Counter)

    def record_result(self, method_name, result):
        self.result_counters[method_name][result] += 1

    def log_result_summary(self, method_name):
        if time.time() >= self.next_summary_time:
            results = []
            for name, val in six.iteritems(vars(FuzzTester.Result)):
                if name.startswith("_"):
                    continue
                count = self.result_counters[method_name][val]
                if count > 0:
                    results.append((name, count))
            results.sort()
            logging.info(
                "%s count: {%s}"
                % (method_name, ", ".join("%s: %d" % r for r in results))
            )

            interval = self.__class__.summary_interval
            # Determine how many full intervals have passed between
            # self.next_summary_time (the scheduled time for this summary) and
            # the time the summary is actually completed.
            intervals_passed = int((time.time() - self.next_summary_time) / interval)
            # Schedule the next summary for the first interval that has not yet
            # fully passed
            self.next_summary_time += interval * (intervals_passed + 1)

    def run(self):
        self.start_logging()
        self.start_timing()
        self.start_result_counters()

        logging.info("Starting Fuzz Tester")
        logging.info(str(self.config))

        self.service = self.config.load_service()

        client_class = self.service.client_class

        methods = self.service.get_methods(self.config.functions)
        constraints = self.preprocess_constraints(self.config.constraints)
        self.method_randomizers = self.get_method_randomizers(methods, constraints)

        logging.info("Fuzzing methods: %s" % methods.keys())

        with FuzzerClient(self.config, client_class) as self.client:
            for method_name, spec in six.iteritems(methods):
                result_spec = spec.get("result_spec", None)
                thrift_exceptions = spec["thrift_exceptions"]
                is_oneway = result_spec is None
                logging.info("Fuzz testing method %s" % (method_name))
                self.n_tests = 0
                self.n_exceptions = 0
                did_crash = False
                for kwargs in self.fuzz_kwargs(method_name, self.config.n_iterations):
                    if not self.run_test(
                        method_name, kwargs, None, is_oneway, thrift_exceptions
                    ):
                        did_crash = True
                        break
                    self.log_result_summary(method_name)
                    self.previous_kwargs = kwargs

                if did_crash:
                    logging.error(
                        ("Method %s caused the " "server to crash.") % (method_name)
                    )
                    break
                else:
                    logging.info(
                        ("Method %s raised unexpected " "exceptions in %d/%d tests.")
                        % (method_name, self.n_exceptions, self.n_tests)
                    )

        self.timer.summarize()


def run_fuzzer(config) -> None:
    fuzzer = FuzzTester(config)
    fuzzer.run()


def fuzz_service(service: Service, ttypes, constants) -> None:
    """Run the tester with required modules input programmatically"""
    service = Service(ttypes, constants, service)
    config = FuzzerConfiguration(service)
    run_fuzzer(config)


if __name__ == "__main__":
    config = FuzzerConfiguration()
    run_fuzzer(config)
