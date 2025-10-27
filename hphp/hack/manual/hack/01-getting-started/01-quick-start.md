# Quick Start

<FbInfo>

Unless you are specifically working on open source Hack code, you want [Facebook's internal documentation](https://our.internmc.facebook.com/intern/wiki/First-app/) instead for dev environment setup. If you're just looking to [learn the Hack language itself](/hack/source-code-fundamentals/introduction), skip this Getting Started section.

</FbInfo>

### 1. Installation

The HHVM package includes everything you need to work with Hack,
including the runtime and typechecker.

See the [HHVM installation page](/hhvm/installation/introduction) to
find the package relevant for your platform.

### 2. Initialize A Project

Create a directory with a `.hhconfig` file in it. This will be the
root of your project.

```
$ mkdir my_project
$ cd my_project
$ touch .hhconfig
```

### 3. Write Your First Hack Program

Create a file called `my_project/hello.hack` with the following code:

```hack
use namespace HH\Lib\IO;

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await IO\request_output()->writeAllAsync("Hello World!\n");
}
```

### 4. Run The Typechecker

Normally you'll get type errors and hover information from within your
IDE. You can also run the typechecker directly to confirm that you
have no type errors in any files in your project.

```bash
$ cd my_project
$ hh_client
No errors!
```

### 5. Run Your Program

HHVM provides the Hack runtime. You can run your program as follows:

```bash
$ cd my_project
$ hhvm hello.hack
Hello World!
```

### 6. Run A Website

Normally you'll start HHVM as a webserver, and it will automatically
pick up any changes to files you make.


```bash
$ cd my_project
$ hhvm -m server -p 8080
```

You can now visit [http://localhost:8080/hello.hack](http://localhost:8080/hello.hack) to see "Hello
World!" in your browser.
