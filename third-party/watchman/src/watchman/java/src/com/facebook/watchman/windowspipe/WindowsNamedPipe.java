/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.windowspipe;

import com.facebook.watchman.WatchmanTransport;
import com.sun.jna.Memory;
import com.sun.jna.platform.win32.WinBase;
import com.sun.jna.platform.win32.WinError;
import com.sun.jna.platform.win32.WinNT;
import com.sun.jna.ptr.IntByReference;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;

/**
 * Implements a {@link WatchmanTransport} backed by a native windows named pipe.
 * This implementation opens a named pipe in overlapped (async) mode.
 * It means that one thread can read and another thread can write at the same time
 * (exactly what {@link com.facebook.watchman.WatchmanConnection} does).
 */
public class WindowsNamedPipe implements WatchmanTransport {
  private static final WindowsNamedPipeLibrary API = WindowsNamedPipeLibrary.INSTANCE;

  private final WinNT.HANDLE pipeHandle;
  private final InputStream in;
  private final OutputStream out;
  // Assumption: reading and writing are async and happen in different threads (see WatchmanConnection).
  // So, each operation (read, write) has its own waitable object.
  private final WinNT.HANDLE readerWaitable;
  private final WinNT.HANDLE writerWaitable;

  public WindowsNamedPipe(String path) throws IOException {
    pipeHandle =
        API.CreateFile(
            path,
            WinNT.GENERIC_READ | WinNT.GENERIC_WRITE,
            0,
            null,
            WinNT.OPEN_EXISTING,
            WinNT.FILE_FLAG_OVERLAPPED,
            null
        );
    if (WinNT.INVALID_HANDLE_VALUE.equals(pipeHandle) ) {
      throw new IOException("Failed to open a named pipe " + path + " error:" + API.GetLastError());
    }

    in = new NamedPipeInputStream();
    out = new NamedPipeOutputStream();

    readerWaitable = API.CreateEvent(null, true, false, null);
    if (readerWaitable == null) {
      throw new IOException("CreateEvent() failed ");
    }

    writerWaitable = API.CreateEvent(null, true, false, null);
    if (writerWaitable == null) {
      throw new IOException("CreateEvent() failed ");
    }
  }


  @Override
  public void close() throws IOException {
    API.CloseHandle(pipeHandle);
    API.CloseHandle(readerWaitable);
    API.CloseHandle(writerWaitable);
  }

  @Override
  public InputStream getInputStream() {
    return in;
  }

  @Override
  public OutputStream getOutputStream() {
    return out;
  }

  private class NamedPipeOutputStream extends OutputStream {
    @Override
    public void write(int b) throws IOException {
      write(new byte[]{(byte) (0xFF & b)});
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
      byte[] data = Arrays.copyOfRange(b, off, off + len);

      WinBase.OVERLAPPED olap = new WinBase.OVERLAPPED();
      olap.hEvent = writerWaitable;
      olap.write();

      boolean immediate = API.WriteFile(pipeHandle, data, len, null, olap.getPointer());
      if (!immediate) {
        if (API.GetLastError() != WinError.ERROR_IO_PENDING) {
          throw new IOException("WriteFile() failed");
        }
      }
      IntByReference written = new IntByReference();
      if (!API.GetOverlappedResult(pipeHandle, olap.getPointer(), written, true)) {
        throw new IOException("GetOverlappedResult() failed for write operation");
      }
      if (written.getValue() != len) {
        throw new IOException("WriteFile() wrote less bytes than requested");
      }
    }
  }

  private class NamedPipeInputStream extends InputStream {

    @Override
    public int read() throws IOException {
      byte[] b = new byte[1];
      read(b);
      return 0xFF & b[0];
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
      Memory readBuffer = new Memory(len);

      WinBase.OVERLAPPED olap = new WinBase.OVERLAPPED();
      olap.hEvent = readerWaitable;
      olap.write();

      boolean immediate = API.ReadFile(pipeHandle, readBuffer, len, null, olap.getPointer());
      if (!immediate) {
        if (API.GetLastError() != WinError.ERROR_IO_PENDING) {
          throw new IOException("ReadFile() failed ");
        }
      }
      IntByReference read = new IntByReference();
      if (!API.GetOverlappedResult(pipeHandle, olap.getPointer(), read, true)) {
        throw new IOException("GetOverlappedResult() failed for read operation");
      }
      if (read.getValue() != len) {
        throw new IOException("ReadFile() read less bytes than requested");
      }
      byte[] byteArray = readBuffer.getByteArray(0, len);
      System.arraycopy(byteArray, 0, b, off, len);
      return len;
    }
  }

}
