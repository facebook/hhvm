// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// This IPC library is used for a worker to communicate with a decl service.
// It avoids all dynamic allocation where possible. The last remaining
// dynamic allocation is when we receive a request, and have to allocate a buffer for the string name.
// Mixed results on whether loopback TCP or unix domain sockets is faster
// https://stackoverflow.com/questions/14973942/tcp-loopback-connection-vs-unix-domain-socket-performance
// I haven't been able to find if it's faster to call "write" multiple times, or
// copy things into a single buffer and call "write" once.
// In any case it likely doesn't matter, since we'll want to switched to some form
// of communication involving shared-memory and busy-waits and maybe futexes.

// Wrapper around libc::read which turns exit code into a Result
unsafe fn read(
    fd: libc::c_int,
    buf: *mut libc::c_void,
    count: libc::size_t,
) -> std::io::Result<Option<()>> {
    let r = libc::read(fd, buf, count);
    if r > 0 {
        // TODO: maybe fail on incomplete read here?
        return Ok(Some(()));
    } else if r < 0 {
        return Err(std::io::Error::last_os_error());
    } else {
        // note: if you specify count=0, it will return Ok(None)
        return Ok(None);
    }
}

// Wrapper around libc::write which turns exit code into a Result
unsafe fn write(
    fd: libc::c_int,
    buf: *const libc::c_void,
    count: libc::size_t,
) -> std::io::Result<libc::ssize_t> {
    let r = libc::write(fd, buf, count);
    if r > 0 {
        // TODO: maybe fail on incomplete write here?
        return Ok(r);
    } else if r < 0 {
        return Err(std::io::Error::last_os_error());
    } else if count == 0 {
        return Ok(0);
    } else {
        return Err(std::io::Error::new(
            std::io::ErrorKind::WriteZero,
            "WriteZero",
        ));
    }
}

// This function sends a request for symbol over FD.
// The wire format is (1) usize for length of symbol name, (2) u32 for kind,
// (3) symbol in UTF8 format
pub fn write_request(fd: std::os::unix::io::RawFd, symbol: &str, kind: u32) -> std::io::Result<()> {
    unsafe {
        // (1) write size of string
        let len: usize = symbol.len();
        write(
            fd,
            (&len as *const usize) as *const libc::c_void,
            std::mem::size_of::<usize>(),
        )?;
        // (2) write kind
        write(
            fd,
            (&kind as *const u32) as *const libc::c_void,
            std::mem::size_of::<u32>(),
        )?;
        // (3) write the string
        write(fd, symbol.as_ptr() as *const libc::c_void, len)?;
        return Ok(());
    }
}

// This function receives a decl request. It returns Ok(None) upon EOF.
// See write_request for the wire format.
pub fn read_request(fd: std::os::unix::io::RawFd) -> std::io::Result<Option<(u32, String)>> {
    unsafe {
        // (1) read size of string
        let mut len = 0usize;
        if read(
            fd,
            (&mut len as *mut usize) as *mut libc::c_void,
            std::mem::size_of::<usize>(),
        )? == None
        {
            return Ok(None);
        }
        // (2) read kind
        let mut kind = 0u32;
        if read(
            fd,
            (&mut kind as *mut u32) as *mut libc::c_void,
            std::mem::size_of::<u32>(),
        )? == None
        {
            return Err(std::io::Error::new(
                std::io::ErrorKind::UnexpectedEof,
                "Unexpected EOF",
            ));
        };
        // (3) read string
        let mut buf = Vec::with_capacity(len);
        if read(fd, buf.as_mut_ptr() as *mut libc::c_void, len)? == None {
            return Err(std::io::Error::new(
                std::io::ErrorKind::UnexpectedEof,
                "Unexpected EOF",
            ));
        }
        let ptr = buf.as_mut_ptr();
        std::mem::forget(buf);
        let s = String::from_raw_parts(ptr, len, len);
        return Ok(Some((kind, s)));
    }
}

// This function writes a decl response over the wire.
// The format is just a single usize, the offset from the start of sharedmem.
pub fn write_response(fd: std::os::unix::io::RawFd, offset: usize) -> std::io::Result<()> {
    unsafe {
        // (1) write usize offset
        libc::write(
            fd,
            (&offset as *const usize) as *const libc::c_void,
            std::mem::size_of::<usize>(),
        );
        return Ok(());
    }
}

// This function reads a decl response over the wire.
// See write_response for the wire format.
pub fn read_response(fd: std::os::unix::io::RawFd) -> std::io::Result<usize> {
    unsafe {
        // (1) read usize offset
        let mut r = 0usize;
        libc::read(
            fd,
            (&mut r as *mut usize) as *mut libc::c_void,
            std::mem::size_of::<usize>(),
        );
        return Ok(r);
    }
}
