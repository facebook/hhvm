use std::io::{BufRead, Read};
use std::path::Path;

pub fn spawn<'a>(
    root: &Path,
    decl_sock_file: &Path,
    cache_directory: &Path,
) -> std::io::Result<usize> {
    println!(
        "buck run //hphp/hack/src/rearchitecture_proposal_1/hh_mapreduce:hh_mapreduce -- prototype --root {} --decl {} --cache {}",
        root.to_string_lossy(),
        decl_sock_file.to_string_lossy(),
        cache_directory.to_string_lossy(),
    );
    let mut child = std::process::Command::new("buck")
        .arg("run")
        .arg("//hphp/hack/src/rearchitecture_proposal_1/hh_mapreduce:hh_mapreduce")
        .arg("--")
        .arg("prototype")
        .arg("--root")
        .arg(root)
        .arg("--decl")
        .arg(decl_sock_file)
        .arg("--cache")
        .arg(cache_directory)
        .stdin(std::process::Stdio::piped())
        .stderr(std::process::Stdio::piped())
        .stdout(std::process::Stdio::piped())
        .spawn()?;
    let stdin = child.stdin.take().unwrap();
    let mut stdout = std::io::BufReader::new(child.stdout.take().unwrap());
    let mut stderr = std::io::BufReader::new(child.stderr.take().unwrap());

    let stderr_thread = std::thread::spawn(move || loop {
        for s in stderr.by_ref().lines() {
            println!("prototype stderr: {}", &s.unwrap());
        }
    });

    let prefix = "Prototype cache base address: 0x";
    let mut base_address = prefix.to_string() + "not found";
    for sr in stdout.by_ref().lines() {
        let s = sr.unwrap();
        println!("prototype stdout: {}", s);
        if s.starts_with(prefix) {
            base_address = s;
            break;
        }
    }
    let base_address = match usize::from_str_radix(&base_address[prefix.len()..].trim_end(), 16) {
        Err(_) => return Err(std::io::Error::from(std::io::ErrorKind::InvalidData)),
        Ok(i) => i,
    };

    let stdout_thread = std::thread::spawn(move || loop {
        for s in stdout.by_ref().lines() {
            println!("prototype stdout: {}", &s.unwrap());
        }
    });

    // If the prototype dies for any reason, we die: our entire cache is now invalid:
    std::thread::spawn(move || {
        let exit_status = match child.wait() {
            Ok(exit_status) => {
                if let Some(code) = exit_status.code() {
                    code.to_string()
                } else {
                    "terminated by signal".to_string()
                }
            }
            Err(e) => e.to_string(),
        };
        println!("prototype died unexpectedly: exit status {}", exit_status);
        std::mem::drop(stdin);
        stderr_thread.join().unwrap();
        stdout_thread.join().unwrap();
        std::process::exit(1);
    });

    return Ok(base_address);
}
