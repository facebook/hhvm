/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.watchman.windowspipe;

import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.platform.win32.WinBase;
import com.sun.jna.platform.win32.WinNT;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.win32.W32APIOptions;

public interface WindowsNamedPipeLibrary extends WinNT {
  WindowsNamedPipeLibrary INSTANCE = (WindowsNamedPipeLibrary) Native.loadLibrary("kernel32",
      WindowsNamedPipeLibrary.class, W32APIOptions.UNICODE_OPTIONS);

  boolean GetOverlappedResult(HANDLE hFile,
                              Pointer lpOverlapped,
                              IntByReference lpNumberOfBytesTransferred,
                              boolean wait);

  boolean ReadFile(HANDLE hFile, Memory pointer, int nNumberOfBytesToRead,
                   IntByReference lpNumberOfBytesRead, Pointer lpOverlapped);

  HANDLE CreateFile(String lpFileName, int dwDesiredAccess, int dwShareMode,
                    WinBase.SECURITY_ATTRIBUTES lpSecurityAttributes,
                    int dwCreationDisposition, int dwFlagsAndAttributes,
                    HANDLE hTemplateFile);

  HANDLE CreateEvent(WinBase.SECURITY_ATTRIBUTES lpEventAttributes,
                     boolean bManualReset, boolean bInitialState, String lpName);

  boolean CloseHandle(HANDLE hObject);

  boolean WriteFile(HANDLE hFile, byte[] lpBuffer, int nNumberOfBytesToWrite,
                    IntByReference lpNumberOfBytesWritten,
                    Pointer lpOverlapped);

  int GetLastError();
}
