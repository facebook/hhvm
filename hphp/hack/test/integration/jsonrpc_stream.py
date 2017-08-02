from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import json
from threading import Thread
from queue import Queue, Empty


class JsonRpcStreamReader:
    def __init__(self, stream):
        self.stream = stream
        self.queue = Queue()
        # daemon ensures the reading thread will get cleaned up when
        # the main program exits.  no need to explicitly manage it.
        self.read_thread = Thread(target=self._async_read_loop, daemon=True)
        self.read_thread.start()

    def read(self):
        return self._read(timeout_seconds=None)

    def try_read(self, timeout_seconds):
        return self._read(timeout_seconds)

    def _async_read_loop(self):
        while True:
            try:
                self.queue.put(json.loads(self._read_payload()))
            except ValueError:
                break
            except IndexError:
                break

    def _read(self, timeout_seconds):
        try:
            return self.queue.get(block=True,
                                  timeout=timeout_seconds)
        except Empty:
            return None

    def _read_content_length(self):
        # read the 'Content-Length:' line and absorb the newline
        # after it
        length_line = self.stream.readline().decode()
        self.stream.read(len("\r\n"))

        # get the content length as an integer for the
        # rest of the package
        parts = length_line.split(":", 1)
        return int(parts[1].strip())

    def _read_content(self, length):
        return self.stream.read(length)

    def _read_payload(self):
        length = self._read_content_length()
        return self._read_content(length)


class JsonRpcStreamWriter:
    def __init__(self, stream):
        self.stream = stream

    def write(self, json_data):
        serialized = json.dumps(json_data)
        content_length = len(serialized)
        payload = "Content-Length: {c}\n\n{s}".format(c=content_length, s=serialized)
        self._write_string(payload)

    def _write_string(self, s):
        self.stream.write(s.encode())
        self.stream.flush()
