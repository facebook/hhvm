#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import errno
import os
import re
import select
import shutil
import signal
import socket
import subprocess
import sys
import tempfile
import time
import ssl

from mcrouter.test.config import McrouterGlobals


class BaseDirectory:
    def __init__(self, prefix="mctest"):
        self.path = tempfile.mkdtemp(prefix=prefix + '.')

    def __del__(self):
        shutil.rmtree(self.path)


def MCPopen(cmd, stdout=None, stderr=None, env=None, pass_fds=()):
    if sys.version_info >= (3, 2):  # Python 3.2 supports pass_fds
        return subprocess.Popen(
            cmd, stdout=stdout, stderr=stderr, env=env, pass_fds=pass_fds)
    return subprocess.Popen(cmd, stdout=stdout, stderr=stderr, env=env)


class ProcessBase:
    """
    Generic process, extended by mcrouter, memcached, mcpiper, etc
    """

    proc = None

    def __init__(self, cmd, base_dir=None, junk_fill=False, pass_fds=()):
        if base_dir is None:
            base_dir = BaseDirectory('ProcessBase')
        self.base_dir = base_dir

        self.cmd_line = 'no command line'
        if cmd:
            self.cmd_line = ' '.join(cmd)
            for command in cmd:
                if command == 'python':
                    continue
                if command.startswith('-'):
                    continue
                command = os.path.basename(command)
                break

            try:
                if junk_fill:
                    env = {'MALLOC_CONF': 'junk:true'}
                else:
                    env = None
                self.proc = MCPopen(cmd, stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE, env=env,
                                    pass_fds=pass_fds)

            except OSError:
                sys.exit("Fatal: Could not run " + repr(" ".join(cmd)))
        else:
            self.proc = None

    def __del__(self):
        if self.proc:
            self.proc.terminate()

    def getprocess(self):
        return self.proc

    def pause(self):
        if self.proc:
            self.proc.send_signal(signal.SIGSTOP)

    def resume(self):
        if self.proc:
            self.proc.send_signal(signal.SIGCONT)

    def is_alive(self):
        self.proc.poll()
        return self.proc.returncode is None

    def get_log(self):
        if hasattr(self, 'log'):
            print(self.base_dir)
            try:
                with open(self.log, 'r') as log_f:
                    log = log_f.read()
            except IOError:
                log = ""
        else:
            log = ""
        return log

    def dump(self):
        """ dump stderr, stdout, and the log file to stdout with nice headers.
        This allows us to get all this information in a test failure (hidden by
        default) so we can debug better. """

        try:
            stdout, stderr = self.proc.communicate()
        except Exception:
            stdout, stderr = b'', b''

        log = self.get_log()
        if log:
            print("{} ({}) log:\n{}".format(self, self.cmd_line, log))
        if stdout:
            print("{} ({}) stdout:\n{}".format(self, self.cmd_line,
                                               stdout.decode()))
        if stderr:
            print("{} ({}) stderr:\n{}".format(self, self.cmd_line,
                                               stderr.decode()))


class MCProcess(ProcessBase):
    proc = None

    def __init__(self, cmd, addr,
            base_dir=None,
            max_retries=100,
            junk_fill=False,
            pass_fds=(),
            use_ssl=False,
            versionPing=False):
        self.fd = None
        self.versionPing = versionPing
        if cmd is not None and '-s' in cmd:
            if os.path.exists(addr):
                raise Exception('file path {} already exists'.format(addr))
            self.addr = addr
            self.port = 0
            self.addr_family = socket.AF_UNIX
        else:
            port = int(addr)
            self.addr = ('localhost', port)
            self.port = port
            self.addr_family = socket.AF_INET
        memcached = False
        if cmd is not None and 'memcached' in cmd[0]:
            memcached = True

        if base_dir is None:
            base_dir = BaseDirectory('MCProcess')

        ProcessBase.__init__(self, cmd, base_dir, junk_fill, pass_fds=pass_fds)
        # memcached could take little longer to initialize
        if memcached:
            time.sleep(3)
        self.max_retries = max_retries
        self.deletes = 0
        self.others = 0
        self.socket = None
        self.use_ssl = use_ssl
        self.fd = None

    def _sendall(self, s):
        if type(s) is not bytes:
            s = s.encode('utf8')
        self.socket.sendall(s)

    def _fdread(self, n):
        data = self.fd.read(n)
        if data is not None and type(data) is not str:
            data = data.decode('utf8')
        return data

    def _fdreadline(self):
        data = self.fd.readline()
        if data is not None and type(data) is not str:
            data = data.decode('utf8')
        return data

    def getport(self):
        return self.port

    def getsslport(self):
        return None

    def connect(self):
        plain_socket = socket.socket(self.addr_family, socket.SOCK_STREAM)
        if self.use_ssl:
            context = ssl.create_default_context()
            # This is for testing, therefore:
            # - We don't need to verify hostname
            # - We are using a self-signed cert
            context.check_hostname = False
            context.verify_mode = ssl.CERT_NONE
            self.socket = context.wrap_socket(plain_socket)
        else:
            self.socket = plain_socket
        self.socket.connect(self.addr)
        self.fd = self.socket.makefile(mode="rb")

    def ensure_connected(self):
        retry_count = 0
        # First, try to connect
        while True:
            try:
                self.connect()
                break
            except Exception as e:
                print(f"Caught exception while connect. errno: {e.errno}")
                self.disconnect()
                if not self.is_alive():
                    print("Process exited unexpectedly!")
                    self.terminate()  # This will print logs as well
                    raise
                retry_count += 1
                # If we defined a retry count, retry until that's exceeded.
                if (not self.max_retries or retry_count < self.max_retries):
                    time.sleep(1)
                    continue
                raise

        # Then, verify Memcache is ready for traffic
        if self.versionPing:
            self._sendall("version\r\n")
            while True:
                fds = select.select([self.fd], [], [], 5.0)
                if len(fds[0]) > 0:
                    line = self._fdreadline()
                    if line.startswith("VERSION"):
                        return
                retry_count += 1
                if self.max_retries and retry_count >= self.max_retries:
                    raise RuntimeError("MCProcess connected but did not respond to ping")

    def disconnect(self):
        try:
            if self.socket:
                self.socket.close()
        except IOError:
            pass
        try:
            if self.fd:
                self.fd.close()
        except IOError:
            pass
        self.fd = self.socket = None

    def terminate(self):
        if not self.proc:
            return None

        if hasattr(self, 'socket'):
            self.disconnect()

        proc = self.proc
        if self.proc:
            if self.proc.returncode is None:
                self.proc.terminate()
            self.proc.wait()

        self.dump()
        self.proc = None

        return proc

    def _get(self, cmd, keys, expect_cas, return_all_info):
        multi = True
        hadValue = False
        if not isinstance(keys, list):
            multi = False
            keys = [keys]
        self._sendall("{} {}\r\n".format(cmd, " ".join(keys)))
        res = {key: None for key in keys}

        while True:
            line = self._fdreadline().strip()
            if line == 'END':
                if multi:
                    return res
                else:
                    assert len(res) == 1
                    return next(v for v in res.values())
            elif line.startswith("VALUE"):
                hadValue = True
                parts = line.split()
                k = parts[1]
                f = int(parts[2])
                n = int(parts[3])
                assert k in keys
                payload = self._fdread(n)
                self._fdread(2)
                if return_all_info:
                    res[k] = dict({"key": k,
                                  "flags": f,
                                  "size": n,
                                  "value": payload})
                    if expect_cas:
                        res[k]["cas"] = int(parts[4])
                else:
                    res[k] = payload
            elif line.startswith("SERVER_ERROR"):
                if hadValue:
                    raise Exception('Received hit reply + SERVER_ERROR for '
                                    'multiget request')
                return line
            else:
                self.connect()
                raise Exception(
                    'Unexpected response "{}" ({})'.format(line, keys))

    def get(self, keys, return_all_info=False):
        return self._get('get', keys, expect_cas=False,
                         return_all_info=return_all_info)

    def gets(self, keys):
        return self._get('gets', keys, expect_cas=True, return_all_info=True)

    def gat(self, exptime, keys, return_all_info=False):
        return self._get('gat ' + str(exptime), keys, expect_cas=False,
                         return_all_info=return_all_info)

    def gats(self, exptime, keys):
        return self._get('gats ' + str(exptime), keys, expect_cas=True,
                         return_all_info=True)

    def metaget(self, keys):
        ## FIXME: Not supporting multi-metaget yet
        #multi = True
        #if not instance(keys, list):
        #    multi = False
        #    keys = [keys]
        res = {}
        self._sendall("metaget {}\r\n".format(keys))

        while True:
            line = self._fdreadline().strip()
            if line.startswith("END"):
                return res
            elif line.startswith("META"):
                meta_list = line.split()
                for i in range(1, len(meta_list) // 2):
                    res[meta_list[2 * i].strip(':')] = \
                        meta_list[2 * i + 1].strip(';')

    def leaseGet(self, keys):
        multi = True
        if not isinstance(keys, list):
            multi = False
            keys = [keys]
        self._sendall("lease-get {}\r\n".format(" ".join(keys)))
        res = {key: None for key in keys}

        while True:
            line = self._fdreadline().strip()
            if line == 'END':
                if multi:
                    assert(len(res) == len(keys))
                    return res
                else:
                    assert len(res) == 1
                    return next(v for v in res.values())
            elif line.startswith("VALUE"):
                v, k, f, n = line.split()
                assert k in keys
                res[k] = {"value": self._fdread(int(n)),
                          "token": None}
                self._fdread(2)
            elif line.startswith("LVALUE"):
                v, k, t, f, n = line.split()
                assert k in keys
                res[k] = {"value": self._fdread(int(n)),
                          "token": int(t)}

    def expectNoReply(self):
        self.socket.settimeout(0.5)
        try:
            self.socket.recv(1)
            return False
        except socket.timeout:
            pass
        return True

    def _set(self, command, key, value, replicate=False, noreply=False,
             exptime=0, flags=0):
        value = str(value)
        flags = flags | (1024 if replicate else 0)
        self._sendall(
            "{command} {key} {flags} {exptime} {size}{noreply}\r\n{value}\r\n"
            .format(
                command=command,
                key=key,
                flags=flags,
                exptime=exptime,
                size=len(value),
                noreply=' noreply' if noreply else '',
                value=value,
            )
        )
        if noreply:
            return self.expectNoReply()

        answer = self._fdreadline().strip()
        if re.search('ERROR', answer):
            print(answer)
            self.connect()
            return None
        return re.match("STORED", answer)

    def leaseSet(self, key, value_token, exptime=0, is_stalestored=False):
        value = str(value_token["value"])
        token = int(value_token["token"])
        flags = 0
        cmd = (
            "lease-set {key} {token} {flags} {exptime} {size}\r\n{value}\r\n"
            .format(
                key=key,
                token=token,
                flags=flags,
                exptime=exptime,
                size=len(value),
                value=value,
            )
        )
        self._sendall(cmd)

        answer = self._fdreadline().strip()
        if re.search('ERROR', answer):
            print(answer)
            self.connect()
            return None
        if is_stalestored:
            return re.match("STALE_STORED", answer)
        return re.match("STORED", answer)

    def set(self, key, value, replicate=False, noreply=False, exptime=0,
            flags=0):
        return self._set("set", key, value, replicate, noreply, exptime, flags)

    def add(self, key, value, replicate=False, noreply=False):
        return self._set("add", key, value, replicate, noreply)

    def replace(self, key, value, replicate=False, noreply=False):
        return self._set("replace", key, value, replicate, noreply)

    def delete(self, key, exptime=None, noreply=False):
        exptime_str = ''
        if exptime is not None:
            exptime_str = " {}".format(exptime)
        self._sendall("delete {}{}{}\r\n".format(
                      key, exptime_str, (' noreply' if noreply else '')))
        self.deletes += 1

        if noreply:
            return self.expectNoReply()

        answer = self._fdreadline()

        assert re.match("DELETED|NOT_FOUND|SERVER_ERROR", answer), answer
        return re.match("DELETED", answer)

    def touch(self, key, exptime, noreply=False):
        self._sendall("touch {} {}{}\r\n".format(
                      key, exptime, (' noreply' if noreply else '')))

        if noreply:
            return self.expectNoReply()

        answer = self._fdreadline()

        if answer == "TOUCHED\r\n":
            return "TOUCHED"
        if answer == "NOT_FOUND\r\n":
            return "NOT_FOUND"
        if re.match("^SERVER_ERROR", answer):
            return "SERVER_ERROR"
        if re.match("^CLIENT_ERROR", answer):
            return "CLIENT_ERROR"
        return None

    def _arith(self, cmd, key, value, noreply):
        self._sendall("{cmd} {key} {value}{noreply}\r\n".format(
            cmd=cmd,
            key=key,
            value=value,
            noreply=' noreply' if noreply else '',
        ))
        if noreply:
            return self.expectNoReply()

        answer = self._fdreadline()
        if re.match("NOT_FOUND", answer):
            return None
        elif "ERROR" in answer:
            return answer
        else:
            return int(answer)

    def incr(self, key, value=1, noreply=False):
        return self._arith('incr', key, value, noreply)

    def decr(self, key, value=1, noreply=False):
        return self._arith('decr', key, value, noreply)

    def _affix(self, cmd, key, value, noreply=False, flags=0, exptime=0):
        self._sendall(
            "{cmd} {key} {flags} {exptime} {size}{noreply}\r\n{value}\r\n"
            .format(
                cmd=cmd,
                key=key,
                flags=flags,
                exptime=exptime,
                size=len(value),
                noreply=' noreply' if noreply else '',
                value=value,
            )
        )

        if noreply:
            return self.expectNoReply()

        answer = self._fdreadline()
        if answer == "STORED\r\n":
            return "STORED"
        if answer == "NOT_STORED\r\n":
            return "NOT_STORED"
        if re.match("^SERVER_ERROR", answer):
            return "SERVER_ERROR"
        if re.match("^CLIENT_ERROR", answer):
            return "CLIENT_ERROR"
        return None

    def append(self, key, value, noreply=False, flags=0, exptime=0):
        return self._affix('append', key, value, noreply, flags, exptime)

    def prepend(self, key, value, noreply=False, flags=0, exptime=0):
        return self._affix('prepend', key, value, noreply, flags, exptime)

    def cas(self, key, value, cas_token):
        value = str(value)
        self._sendall("cas {key} 0 0 {size} {token}\r\n{value}\r\n".format(
            key=key,
            size=len(value),
            token=cas_token,
            value=value,
        ))

        answer = self._fdreadline().strip()
        if re.search('ERROR', answer):
            print(answer)
            self.connect()
            return None
        return re.match("STORED", answer)

    def stats(self, spec=None):
        q = 'stats\r\n'
        if spec:
            q = 'stats {spec}\r\n'.format(spec=spec)
        self._sendall(q)

        s = {}
        line = None
        fds = select.select([self.fd], [], [], 20.0)
        if len(fds[0]) == 0:
            return None
        while line != 'END':
            line = self._fdreadline().strip()
            if len(line) == 0:
                return None
            a = line.split(None, 2)
            if len(a) == 3:
                s[a[1]] = a[2]

        return s

    def raw_stats(self, spec=None):
        q = 'stats\r\n'
        if spec:
            q = 'stats {spec}\r\n'.format(spec=spec)
        self._sendall(q)

        s = []
        line = None
        fds = select.select([self.fd], [], [], 20.0)
        if len(fds[0]) == 0:
            self.terminate()
            raise RuntimeError("MCProcess failed to return stats")
        while line != 'END':
            line = self._fdreadline().strip()
            a = line.split(None, 1)
            if len(a) == 2:
                s.append(a[1])

        return s

    def issue_command_and_read_all(self, command):
        self.others += 1
        self._sendall(command)

        # Handle no response
        fds = select.select([self.fd], [], [], 20.0)
        if len(fds[0]) == 0:
            return None

        answer = ""
        line = None
        while line != 'END':
            line = self._fdreadline().strip()
            # Handle error
            if not answer and 'ERROR' in line:
                self.connect()
                return line
            answer += line + "\r\n"
        return answer

    def issue_command(self, command):
        self.others += 1
        self._sendall(command)
        answer = self._fdreadline()
        return answer

    def version(self):
        self._sendall("version\r\n")
        return self._fdreadline()

    def shutdown(self):
        self._sendall("shutdown\r\n")
        return self._fdreadline()

    def flush_all(self, delay=None):
        if delay is None:
            self._sendall("flush_all\r\n")
        else:
            self._sendall("flush_all {}\r\n".format(delay))
        return self._fdreadline().rstrip()


ATTR_PAT = '(?P<attr>(:([0-9a-z_])+){0,3})'
IPV6_PAT = '\\[([A-Za-z:0-9]+)\\]'
IPV6_SERVER_PAT = re.compile(f'^(?P<host>{IPV6_PAT}):(?P<port>[0-9]+){ATTR_PAT}$')
SERVER_PAT = re.compile(f'^(?P<host>[0-9a-zA-Z_.]+):(?P<port>[0-9]+){ATTR_PAT}$')


def parse_parts(s, pattern):
    m = pattern.fullmatch(s)
    if m:
        parts = [m["host"], m["port"]]
        if m["attr"]:
            parts.append(m["attr"].lstrip(":"))
        return parts
    return None


def sub_port(s, substitute_ports, port_map):

    parts = parse_parts(s, IPV6_SERVER_PAT) or parse_parts(s, SERVER_PAT)
    if parts:
        try:
            port = int(parts[1])
            if port not in port_map:
                if len(port_map) < len(substitute_ports):
                    if isinstance(substitute_ports, list):
                        port_map[port] = substitute_ports[len(port_map)]
                    else:
                        if port not in substitute_ports:
                            raise Exception(
                                "Port {} not in substitute port map"
                                .format(port))
                        port_map[port] = substitute_ports[port]
                else:
                    raise Exception("Looking up port {}: config file has more "
                                    "ports specified than the number of "
                                    "mock servers started".format(port))
            parts[1] = str(port_map[port])
            return ':'.join(parts)
        except (IndexError, ValueError):
            pass
    return s


def replace_ports(json, substitute_ports):
    """In string json (which must be a valid JSON string), replace all ports in
    strings of the form "host:port" with ports from the list or map
    substitute_ports.

    If list, each new distinct port from the json will be replaced from the
    next port from the list.

    If map of the form (old_port: new_port), replaces all old_ports with
    new_ports.
    """
    NORMAL = 0
    STRING = 1
    ESCAPE = 2

    state = NORMAL
    out = ""
    s = ""
    port_map = {}
    for c in json:
        if state == NORMAL:
            out += c
            if c == '"':
                s = ""
                state = STRING
        elif state == STRING:
            if c == '\\':
                s += c
                state = ESCAPE
            elif c == '"':
                out += sub_port(s, substitute_ports, port_map)
                out += c
                state = NORMAL
            else:
                s += c
        elif state == ESCAPE:
            s += c
            state = NORMAL

    if len(port_map) < len(substitute_ports):
        raise Exception("Config file has fewer ports specified than the number"
                        " of mock servers started")
    return out


def replace_strings(json, replace_map):
    for (key, value) in replace_map.items():
        json = json.replace(key, str(value))
    return json


def create_listen_socket():
    if socket.has_ipv6:
        listen_sock = socket.socket(socket.AF_INET6)
    else:
        listen_sock = socket.socket(socket.AF_INET)
    listen_sock.listen(100)
    return listen_sock


class McrouterBase(MCProcess):
    def __init__(self, args, port=None, base_dir=None):
        if base_dir is None:
            base_dir = BaseDirectory('mcrouter')

        self.log = os.path.join(base_dir.path, 'mcrouter.log')

        self.async_spool = os.path.join(base_dir.path, 'spool.mcrouter')
        os.mkdir(self.async_spool)
        self.stats_dir = os.path.join(base_dir.path, 'stats')
        os.mkdir(self.stats_dir)
        self.debug_fifo_root = os.path.join(base_dir.path, 'fifos')
        os.mkdir(self.debug_fifo_root)

        args.extend(['-L', self.log,
                     '-a', self.async_spool,
                     '--stats-root', self.stats_dir,
                     '--debug-fifo-root', self.debug_fifo_root,
                     '--rss-limit-mb', '16384'])

        listen_sock = None
        pass_fds = []
        if port is None:
            listen_sock = create_listen_socket()
            port = listen_sock.getsockname()[1]
            listen_sock_fd = listen_sock.fileno()
            args.extend(['--listen-sock-fd', str(listen_sock_fd)])
            pass_fds.append(listen_sock_fd)
        else:
            args.extend(['-p', str(port)])

        args = McrouterGlobals.preprocessArgs(args)

        MCProcess.__init__(self, args, port, base_dir, junk_fill=True,
                           pass_fds=pass_fds)

        if listen_sock is not None:
            listen_sock.close()

    def get_async_spool_dir(self):
        return self.async_spool

    def check_in_log(self, needle):
        return needle in open(self.log).read()


class Mcrouter(McrouterBase):
    def __init__(self, config, port=None, default_route=None, extra_args=None,  # noqa: C901
                 base_dir=None, substitute_config_ports=None, substitute_config_smc_ports=None,
                 substitute_port_map=None, replace_map=None, flavor=None,
                 sr_mock_smc_config=None):
        if base_dir is None:
            base_dir = BaseDirectory('mcrouter')

        if replace_map:
            with open(config, 'r') as config_file:
                replaced_config = replace_strings(config_file.read(),
                                                  replace_map)
            (_, config) = tempfile.mkstemp(dir=base_dir.path)
            with open(config, 'w') as config_file:
                config_file.write(replaced_config)

        if substitute_config_ports:
            with open(config, 'r') as config_file:
                replaced_config = replace_ports(config_file.read(),
                                                substitute_config_ports)
            (_, config) = tempfile.mkstemp(dir=base_dir.path)
            with open(config, 'w') as config_file:
                config_file.write(replaced_config)

        self.config = config
        if (flavor):
            args = [McrouterGlobals.binPath('mcrouter'), 'file:' + flavor,
                    '--config', 'file:' + config]
        else:
            args = [McrouterGlobals.binPath('mcrouter'),
                    '--config', 'file:' + config]

        if default_route:
            args.extend(['-R', default_route])

        if extra_args:
            args.extend(extra_args)

        if '-b' in args:
            def get_pid():
                stats = self.stats()
                if stats:
                    return int(stats['pid'])
                return None

            def terminate():
                pid = get_pid()
                if pid:
                    os.kill(pid, signal.SIGTERM)

            def is_alive():
                pid = get_pid()
                if pid:
                    return os.path.exists("/proc/{}".format(pid))
                return False

            self.terminate = terminate
            self.is_alive = is_alive

        if substitute_config_smc_ports and sr_mock_smc_config:
            with open(sr_mock_smc_config, 'r') as config_file:
                replaced_config = replace_ports(config_file.read(),
                                                substitute_config_smc_ports)
            (_, sr_mock_smc_config) = tempfile.mkstemp(dir=base_dir.path)
            with open(sr_mock_smc_config, 'w') as config_file:
                config_file.write(replaced_config)
            self.sr_mock_smc_config = "file:" +sr_mock_smc_config
            args.extend(['--debug-sr-host-list', self.sr_mock_smc_config])

        McrouterBase.__init__(self, args, port, base_dir)

    def change_config(self, new_config_path):
        shutil.copyfile(new_config_path, self.config)


class McrouterClient(MCProcess):
    def __init__(self, port):
        MCProcess.__init__(self, None, str(port))


class McrouterClients:
    def __init__(self, port, count):
        self.clients = []
        for i in range(0, count):
            self.clients.append(McrouterClient(port))
            self.clients[i].connect()

    def __getitem__(self, idx):
        return self.clients[idx]


class MockMemcached(MCProcess):
    def __init__(self, port=None):
        args = [McrouterGlobals.binPath('mockmc')]
        listen_sock = None
        pass_fds = []
        if port is None:
            listen_sock = create_listen_socket()
            port = listen_sock.getsockname()[1]
            listen_sock_fd = listen_sock.fileno()
            args.extend(['-t', str(listen_sock_fd)])
            pass_fds.append(listen_sock_fd)
        else:
            args.extend(['-P', str(port)])

        MCProcess.__init__(self, args, port, pass_fds=pass_fds)

        if listen_sock is not None:
            listen_sock.close()


class MockMemcachedThrift(MCProcess):
    def __init__(self, port=None):
        args = [McrouterGlobals.binPath('mockmcthrift')]
        listen_sock = None
        pass_fds = []
        if port is None:
            listen_sock = create_listen_socket()
            port = listen_sock.getsockname()[1]
            listen_sock_fd = listen_sock.fileno()
            args.extend(['-t', str(listen_sock.fileno())])
            pass_fds.append(listen_sock_fd)
        else:
            args.extend(['-P', str(port)])

        MCProcess.__init__(self, args, port, pass_fds=pass_fds)

        if listen_sock is not None:
            listen_sock.close()


class MockMemcachedDual(MCProcess):
    def __init__(self, thriftPort=None, asyncPort=None, extra_args=None, mcrouterUseThrift=True):
        args = [McrouterGlobals.binPath('mockmcdual')]
        pass_fds = []
        if thriftPort is None:
            listenSocketThrift = create_listen_socket()
            thriftPort = listenSocketThrift.getsockname()[1]
            sock_fd = listenSocketThrift.fileno()
            args.extend(['-t', str(sock_fd)])
            pass_fds.append(sock_fd)
        else:
            args.extend(['-p', str(thriftPort)])
        self.thriftPort = thriftPort
        self.mcrouterUseThrift = mcrouterUseThrift

        if asyncPort is None:
            listenSocketAsyncMc = create_listen_socket()
            asyncPort = listenSocketAsyncMc.getsockname()[1]
            sock_fd = listenSocketAsyncMc.fileno()
            args.extend(['-T', str(sock_fd)])
            pass_fds.append(sock_fd)
        else:
            args.extend(['-P', str(asyncPort)])
        self.asyncPort = asyncPort

        if extra_args:
            args.extend(extra_args)

        MCProcess.__init__(self, args, asyncPort, pass_fds=pass_fds)

        if listenSocketThrift is not None:
            listenSocketThrift.close()
        if listenSocketAsyncMc is not None:
            listenSocketAsyncMc.close()

    def getport(self):
        return self.thriftPort if self.mcrouterUseThrift else self.asyncPort


class Memcached(MCProcess):
    ssl_port = None

    def __init__(self, port=None, ssl_port=None, extra_args=None):
        args = [McrouterGlobals.binPath('prodmc')]
        listen_sock = None
        pass_fds = []

        # if mockmc is used here, we initialize the same way as MockMemcached
        if McrouterGlobals.binPath('mockmc') == args[0]:
            if port is None:
                listen_sock = create_listen_socket()
                port = listen_sock.getsockname()[1]
                listen_sock_fd = listen_sock.fileno()
                args.extend(['-t', str(listen_sock_fd)])
                pass_fds.append(listen_sock_fd)
            else:
                args.extend(['-P', str(port)])

            MCProcess.__init__(self, args, port, pass_fds=pass_fds)

            if listen_sock is not None:
                listen_sock.close()
        else:
            args.extend([
                '-A',
                '-g',
                '-t', '1',
                '--enable_hash_aliases',
                '--enable_unchecked_l1_sentinel_reads',
                '--reaper_throttle=100',
                '--ini_hashpower=16',
                '--num_listening_sockets=1',
            ])
            if (extra_args):
                args.extend(extra_args)
            if port is None:
                listen_sock = create_listen_socket()
                port = listen_sock.getsockname()[1]
                listen_sock_fd = listen_sock.fileno()
                args.extend(['--listen_sock_fd', str(listen_sock_fd)])
                pass_fds.append(listen_sock_fd)
            else:
                args.extend(['-p', str(port)])

            # Create a listen thrift socket to avoid port collision.
            # Not used in actual ASCII tests
            thrift_listen_sock = create_listen_socket()
            thrift_listen_sock_fd = thrift_listen_sock.fileno()
            args.extend(['--thrift_listen_sock_fd', str(thrift_listen_sock_fd)])
            pass_fds.append(thrift_listen_sock_fd)

            if ssl_port:
                self.ssl_port = ssl_port
                args.extend(['--ssl_port', str(self.ssl_port)])

            MCProcess.__init__(self, args, port, pass_fds=pass_fds)

            thrift_listen_sock.close()

            if listen_sock is not None:
                listen_sock.close()

            # delay here until the server goes up
            self.ensure_connected()
            tries = 10
            s = self.stats()
            while ((not s or len(s) == 0) and tries > 0):
                # Note, we need to reconnect, because it's possible the
                # server is still going to process previous requests.
                self.ensure_connected()
                s = self.stats()
                time.sleep(0.5)
                tries -= 1
            self.disconnect()

    def getsslport(self):
        return self.ssl_port


class Mcpiper(ProcessBase):
    def __init__(self, fifos_dir, extra_args=None):
        base_dir = BaseDirectory('mcpiper')
        args = [McrouterGlobals.binPath('mcpiper'), '--fifo-root', fifos_dir]

        if extra_args:
            args.extend(extra_args)

        super().__init__(args, base_dir)

    def output(self):
        if not hasattr(self, 'stdout'):
            self.proc.terminate()
            self.stdout = self.proc.stdout.read().decode('ascii', errors='ignore')
        return self.stdout

    def contains(self, needle):
        return needle in self.output()
