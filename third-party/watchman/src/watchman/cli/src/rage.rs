/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fs::DirEntry;
use std::fs::File;
use std::io;
use std::io::BufRead;
use std::io::BufReader;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;
use duct::cmd;
use structopt::StructOpt;
use sysinfo::System;
use sysinfo::SystemExt;
#[cfg(target_os = "linux")]
use tabular::row;
#[cfg(target_os = "linux")]
use tabular::Table;

#[cfg(unix)]
fn getuid() -> nix::unistd::Uid {
    nix::unistd::getuid()
}

fn print_system_info() {
    macro_rules! print_or_unknown {
        ($fmt: expr, $value: expr) => {
            println!($fmt, $value.as_deref().unwrap_or("<unknown>"));
        };
    }

    let mut system = System::new();
    system.refresh_system();

    // Note: OS & Arch here are set at compile time, which can be inaccurate in some cases (Apple Silicon)
    println!("Platform: {}", std::env::consts::OS);
    println!("Arch: {}", std::env::consts::ARCH);
    print_or_unknown!("Hostname: {}", system.host_name());
    print_or_unknown!("Release: {}", system.name());
    print_or_unknown!("System Version: {}", system.os_version());
    print_or_unknown!("Kernel Version: {}", system.kernel_version());
    #[cfg(unix)]
    println!("Running watchman-diag as UID: {}", getuid());
}

#[cfg(unix)]
fn print_package_version() -> Result<()> {
    let version = cmd!("rpm", "-q", "fb-watchman").read()?;
    println!("RPM version (rpm -q fb-watchman): {}", version);
    Ok(())
}

#[cfg(windows)]
fn print_package_version() -> Result<()> {
    let version = cmd!("clist", "-lr", "fb.watchman").read()?;
    println!("Chocolatey version (clist -lr fb.watchman): {}", version);
    Ok(())
}

fn print_cli_version() -> Result<()> {
    let version = cmd!("watchman", "--no-spawn", "-v").read()?;
    println!("CLI version (watchman -v): {}", version);
    Ok(())
}

fn print_watchman_env() -> Result<()> {
    let vars = std::env::vars()
        .into_iter()
        .filter(|(k, _)| k.starts_with("WATCHMAN_"))
        .collect::<Vec<_>>();

    if !vars.is_empty() {
        println!(
            "WARNING: The following Watchman related environment variables are set (this is unusual and may cause problems):"
        );

        for (k, v) in vars.iter() {
            println!("{}={}", k, v);
        }
    }

    Ok(())
}

#[cfg(target_os = "linux")]
fn print_inotify() -> Result<()> {
    let mut table = Table::new("{:<} {:<} {:<} {:<}").with_row(row!("PID", "EXE", "FD", "WATCHES"));

    macro_rules! bail {
        ($e: expr) => {
            match $e {
                Ok(v) => v,
                Err(_) => continue,
            }
        };
    }
    let procs = match std::fs::read_dir("/proc") {
        Ok(c) => c,
        Err(e) => {
            println!("Unable to crawl inotify information: {:?}", e);
            return Ok(());
        }
    };
    for proc in procs {
        let proc = bail!(proc).path();
        let content = bail!(std::fs::read_dir(proc.join("fd")));
        let exe = std::fs::read_link(proc.join("exe")).map(|x| x.display().to_string());
        let exe = exe.as_deref().unwrap_or("<unknown>");

        for entry in content {
            let fd = bail!(entry).path();
            let target = bail!(std::fs::read_link(&fd));
            if target.display().to_string() != "anon_inode:inotify" {
                continue;
            }

            if let Some(fdnum) = fd.file_name() {
                let fdinfo = proc.join("fdinfo").join(fdnum);
                let fdinfo = bail!(std::fs::read_to_string(fdinfo));
                let fdnum = fdnum.to_string_lossy();
                let pid = proc
                    .file_name()
                    .map_or_else(|| "<unknown>".into(), |x| x.to_string_lossy());

                if fdinfo.is_empty() {
                    table.add_row(row!(pid, exe, fdnum, "<unknown>"));
                } else {
                    let watches = fdinfo
                        .lines()
                        .filter(|line| line.starts_with("inotify wd:"))
                        .count();
                    table.add_row(row!(pid, exe, fdnum, watches));
                }
            }
        }
    }

    println!("Inotify watch information");
    println!("{}", table);
    Ok(())
}

#[cfg(target_os = "macos")]
fn print_launchd_info() -> Result<()> {
    let result = cmd!("launchctl", "list", "com.github.facebook.watchman").read()?;
    println!("Launchd info:");
    println!("{}", result);
    Ok(())
}

fn print_state_info() -> Result<()> {
    // construct all possible state directories
    let mut roots: Vec<PathBuf> = vec![
        "/var/facebook/watchman".into(),
        "/opt/facebook/var/run/watchman".into(),
        "/opt/facebook/watchman/var/run/watchman".into(),
    ];

    if let Ok(path) = std::env::var("TEMP") {
        roots.push(path.into());
    }
    if let Ok(path) = std::env::var("TMP") {
        roots.push(path.into());
    }

    /// Are there any file ends with `-state` name in there
    fn get_state_dirs(p: &Path) -> Vec<DirEntry> {
        if let Ok(content) = std::fs::read_dir(p) {
            content
                .filter_map(|entry| {
                    if let Ok(entry) = entry {
                        if entry.file_name().to_string_lossy().ends_with("-state") {
                            Some(entry)
                        } else {
                            None
                        }
                    } else {
                        None
                    }
                })
                .collect::<Vec<_>>()
        } else {
            Vec::new()
        }
    }

    fn print_state_dir(state: &Path) {
        let state_file = state.join("state");
        println!("State information from {}\n", state.display());
        println!("State file: {}", state_file.display());

        if let Ok(content) = std::fs::read_to_string(state_file) {
            println!("{}", content);
        }

        let log_file = state.join("log");
        if let Ok(lines) = {
            File::open(&log_file).and_then(|file| {
                // We prefer `.split` here instead of `.lines` since the log
                // file may contain invalid utf-8 characters.
                BufReader::new(file)
                    .split(b'\n')
                    .collect::<io::Result<Vec<_>>>()
            })
        } {
            println!("Log samples: {}", log_file.display());

            for line in lines.into_iter().rev().take(300).rev() {
                println!("{}", String::from_utf8_lossy(&line).trim())
            }
        }
    }

    for root in roots.iter() {
        let dirs = get_state_dirs(root);
        for state in dirs {
            print_state_dir(&state.path());
        }
    }

    #[cfg(windows)]
    if let Ok(path) = std::env::var("LOCALAPPDATA") {
        let windows_state = PathBuf::from(&path).join("watchman");
        print_state_dir(&windows_state);
    }

    Ok(())
}

#[cfg(any(target_os = "linux", target_os = "macos"))]
fn print_running_watchman() -> Result<()> {
    let lines = cmd!("ps", "-ef").read()?;

    println!("Running Watchman Processes");
    for line in lines.lines() {
        if line.contains("watchman") {
            println!("{}", line);
        }
    }

    Ok(())
}

#[derive(StructOpt, Debug)]
pub(crate) struct RageCmd {}

impl RageCmd {
    pub(crate) async fn run(&self) -> Result<()> {
        print_system_info();
        println!();
        print_package_version().ok();
        println!();
        print_cli_version().ok();
        println!();
        print_watchman_env().ok();
        println!();
        #[cfg(target_os = "linux")]
        {
            print_inotify().ok();
            println!();
        }
        #[cfg(target_os = "macos")]
        {
            print_launchd_info().ok();
            println!();
        }
        print_state_info().ok();
        println!();
        #[cfg(any(target_os = "linux", target_os = "macos"))]
        {
            print_running_watchman().ok();
            println!();
        }

        Ok(())
    }
}
