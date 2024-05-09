/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#![cfg(windows)]
use core::ffi::c_void;
use std::io::Error as IoError;
use std::os::windows::ffi::OsStrExt;
use std::path::PathBuf;
use std::pin::Pin;
use std::task::Context;
use std::task::Poll;

use tokio::io::AsyncRead;
use tokio::io::AsyncWrite;
use tokio::io::ReadBuf;
use tokio::net::windows::named_pipe::NamedPipeClient;
use winapi::um::fileapi::CreateFileW;
use winapi::um::fileapi::OPEN_EXISTING;
use winapi::um::winbase::FILE_FLAG_OVERLAPPED;
use winapi::um::winnt::GENERIC_READ;
use winapi::um::winnt::GENERIC_WRITE;

use crate::Error;

/// Wrapper around a tokio [`NamedPipeClient`]
pub struct NamedPipe {
    io: NamedPipeClient,
}

impl NamedPipe {
    pub async fn connect(path: PathBuf) -> Result<Self, Error> {
        let win_path = path
            .as_os_str()
            .encode_wide()
            .chain(Some(0))
            .collect::<Vec<_>>();

        let handle = unsafe {
            CreateFileW(
                win_path.as_ptr(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                std::ptr::null_mut(),
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                std::ptr::null_mut(),
            )
        };
        if handle.is_null() {
            let err = IoError::last_os_error();
            return Err(Error::Connect {
                endpoint: path,
                source: Box::new(err),
            });
        }

        let io = unsafe {
            // CreateFileW returns HANDLE which is typedef PVOID HANDLE which itself is typedef void *PVOID;
            // See https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types
            // tokio expects this: pub type HANDLE = *mut c_void;
            NamedPipeClient::from_raw_handle(handle as *mut c_void).map_err(|err| {
                Error::Connect {
                    endpoint: path,
                    source: Box::new(err),
                }
            })?
        };
        Ok(Self { io })
    }
}

impl AsyncRead for NamedPipe {
    fn poll_read(
        self: Pin<&mut Self>,
        ctx: &mut Context,
        buf: &mut ReadBuf,
    ) -> Poll<Result<(), IoError>> {
        AsyncRead::poll_read(Pin::new(&mut self.get_mut().io), ctx, buf)
    }
}

impl AsyncWrite for NamedPipe {
    fn poll_write(
        self: Pin<&mut Self>,
        ctx: &mut Context,
        buf: &[u8],
    ) -> Poll<Result<usize, IoError>> {
        AsyncWrite::poll_write(Pin::new(&mut self.get_mut().io), ctx, buf)
    }

    fn poll_flush(self: Pin<&mut Self>, ctx: &mut Context) -> Poll<Result<(), IoError>> {
        AsyncWrite::poll_flush(Pin::new(&mut self.get_mut().io), ctx)
    }

    fn poll_shutdown(self: Pin<&mut Self>, ctx: &mut Context) -> Poll<Result<(), IoError>> {
        AsyncWrite::poll_shutdown(Pin::new(&mut self.get_mut().io), ctx)
    }
}

impl crate::ReadWriteStream for NamedPipe {}
