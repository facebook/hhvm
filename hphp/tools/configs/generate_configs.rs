/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

/*

This is the script that parses the configuration specification. The format is
following:

```
Any comments until we hit the first section

# SectionName

  Optional section comment paragraph that should be indented 2 spaces

- Type ConfigName[ = DefaultValue], Owner[, feature1|feature2...]

  Optional config comment paragraph that should be indented 2 spaces

```

Supported Types can be found in ConfigType

The ConfigName always need to be prefixed with the `SectionName.`. This is
done to make it easier to search

If no default value is applied you need to implement a custom function that
returns the default value. You can look at php7-impl.cpp for an example.

If the Owner is unknown use `UNKNOWN` as the owner.

Possible features are:
- private, will make the config private so it can only be used as default value
  for other configs in the same section
- globaldata(options), if the config should be part of RepoGlobalData.
  options is a pipe separated list of options.
  - noload: Do not load it in RepoGlobalData::load
- unitcacheflag, if the config should be part of the unit cache hash key
- repooptionsflag(name[, systemlibdefault]), if the config can be set in
  hhvmconfig.hdf inside the repo that contains the hack code. Use systemlibdefault
  incase the normal default is not a constant value
- compileroption(name), if the config is something the compiler needs
- lookuppath(name), use if you need to read from a different name then the configs
  name
- nobind, doesn't call Config::Bind which means it can't be set using hdf or
  command line
- postprocess, if the config needs a PostProcess method to process the value
  after it been read from the config
- staticdefault(value), if you need to set the default of the static variable
  to something other than the default for that type

*/

use std::fs;
use std::path::PathBuf;
use std::process::ExitCode;

use clap::Parser;
use clap::ValueEnum;
use nom::Err;
use nom::error::VerboseError;
use nom::error::convert_error;

#[derive(Debug, Clone, ValueEnum)]
enum OutputType {
    Defs,
    Loader,
    Hackc,
}

#[derive(Debug, Parser)]
#[clap(name = "HHVM Generate Configs")]
struct Arguments {
    #[clap(value_enum)]
    output_type: OutputType,
    output_dir: PathBuf,
    input: PathBuf,
}

fn main() -> ExitCode {
    let args = Arguments::parse();

    let contents = fs::read_to_string(args.input).expect("Should have been able to read the file");

    let res = generate_configs_lib::parse_option_doc::<VerboseError<&str>>(&contents);
    match res {
        Ok((_, sections)) => {
            match args.output_type {
                OutputType::Defs => generate_configs_lib::generate_defs(sections, args.output_dir),
                OutputType::Loader => {
                    generate_configs_lib::generate_loader(sections, args.output_dir)
                }
                OutputType::Hackc => {
                    generate_configs_lib::generate_hackc(sections, args.output_dir)
                }
            }
            ExitCode::from(0)
        }
        Err(Err::Error(e)) | Err(Err::Failure(e)) => {
            println!(
                "error parsing header: {}",
                convert_error(contents.as_str(), e)
            );
            ExitCode::from(1)
        }
        Err(Err::Incomplete(_)) => ExitCode::from(2),
    }
}
