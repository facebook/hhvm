pub fn handle(stream: std::os::unix::net::UnixStream) {
    println!("decl: received incoming connection");
    let fd = std::os::unix::io::AsRawFd::as_raw_fd(&stream);
    loop {
        if let Some((kind, req)) = decl_ipc::read_request(fd).unwrap() {
            println!("decl: received request: kind={} req={}", kind, req);
            decl_ipc::write_response(fd, 0x4000).unwrap();
            println!("decl: sent response");
        } else {
            break;
        }
    }
    println!("decl: closing incoming connection");
}
