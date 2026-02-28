# Introduction

After [installing HHVM](/hhvm/installation/introduction), you will want to configure it to run scripts from the command line and/or serve web traffic.

## Configuration Options

HHVM has a very large set of [configuration options](/hhvm/configuration/INI-settings). Many are not meant for the end HHVM user, but there are some key options that will be useful for anyone deploying HHVM.

### INI Format

HHVM uses configuration files in [INI format](https://en.wikipedia.org/wiki/INI_file). In an INI file, each line represents a configuration in key/value format, where the key is the name of the option, while the value is the value for that option. For example,

```
hhvm.force_hh = 1
hhvm.server_variables[MY_VARIABLE] = "Hello"
```

These settings can be specified in one of two places, or in a combination of both:

* A configuration file, normally suffixed with `.ini` (e.g., `config.ini`)
* At the command line using the `-d` flag to the HHVM binary.

```
hhvm -c config.ini file.php
hhvm -d hhvm.force_hh = 1 file.php
hhvm -c config.ini -d hhvm.log.file = /tmp/temp.log -d hhvm.force_hh = 1 file.php
```

If the same option is specified more than once, then the option that HHVM reads last will be the value used. HHVM reads the command line left to right and INI configuration files top to bottom.

Check out our [INI settings page](/hhvm/configuration/INI-settings#common-options) for the common configuration options you are likely to use day-to-day.
