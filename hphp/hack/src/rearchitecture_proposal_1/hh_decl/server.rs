// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use failure_ext::Result;
use nix::unistd;
use serde::de::DeserializeOwned;
use serde::Serialize;
use std::io::{BufRead, BufReader, Write};
use std::os::unix::net::{UnixListener, UnixStream};
use std::path::Path;

/// The Service abstraction bridges high-level service semantics with the low
/// level details of the IPC transport. It has two associated types:
///   1) Request, the type of incoming requests, and
///   2) Response, the type of outgoing requests.
///
/// The only requirement imposed by Service is that the two be deserializable
/// and serializable, respectively.
pub trait Service: Default {
    type Request: DeserializeOwned;
    type Response: Serialize;

    fn serve(&self, request: &Self::Request) -> Result<Self::Response>;
}

/// UnixDomainServer serves a service as JSON over a Unix domain docket.
pub struct UnixDomainServer<S> {
    server: S,
    listener: UnixListener,
}

// Note: Service must be Send + Sync + 'Static. Read this as:
//  Send - Must be legal to send T across threads
//  Sync - Must be legal to send &T across threads
//  'static - T contains references only to objects of static lifetime
//
// This implicitly pushes the burden of thread safety onto implementors of
// Service.

impl<S: Service + Send + Sync + 'static> UnixDomainServer<S> {
    /// Creates a new UnixDomainServer listening on the given path. Unlinks the
    /// path if a file already exists there.
    pub fn new(path: &Path) -> Result<Self> {
        if path.exists() {
            unistd::unlink(path)?;
        }

        let listener = UnixListener::bind(path)?;
        let server: S = Default::default();
        Ok(UnixDomainServer { server, listener })
    }

    /// Starts the UnixDomainServer on the calling thread.
    pub fn start(self) {
        // These three local variables (listener, service, and service_ref) are
        // all here to help Rust understand what we're doing, since it can't
        // infer it.
        //
        // listener and service move out of self, so self can't be used at all
        // afterwards. service_ref is a hint to the Rust compiler that we want
        // the second of the two below closures to capture service by reference
        // and everything else (i.e. socket) by move.
        let listener = self.listener;
        let service = self.server;

        // We use the `rayon` library here instead of `thread::spawn` because
        // `rayon::scope` allows the by-reference capture of objects on the
        // stack on closures that run on other threads. `rayon::scope` ensures
        // that all spawned threads terminate before `rayon::scope` returns.
        rayon::scope(|t| {
            let ref service_ref = service;
            loop {
                let (socket, _) = listener.accept().unwrap();
                t.spawn(move |_| {
                    handle_connection(socket, service_ref).expect("failed to handle connection");
                });
            }
        });
    }
}

/// Handles a single connection on a Unix stream (or really, any kind of
/// full-duplex stream). handle_connection uses a Service to turn a request
/// into a response. It is responsible for grabbing bytes off the wire and
/// turning the response back into bytes on the wire.
fn handle_connection<S: Service>(socket: UnixStream, server: &S) -> Result<()> {
    let mut reader = BufReader::new(socket.try_clone()?);
    let mut writer = socket;
    loop {
        let mut request_buf = String::new();
        reader.read_line(&mut request_buf)?;
        let req: <S as Service>::Request = serde_json::from_str(&request_buf)?;
        let resp = server.serve(&req)?;
        let json = serde_json::to_string(&resp)?;
        writeln!(&mut writer, "{}", json)?;
    }
}
