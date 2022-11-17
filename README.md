# Sleipnir — DeepSource C++ linter

<br />
<p align="center">
	<a href="#">
		<p align="center" >
		<a href="https://commons.wikimedia.org/wiki/File:Odin,_Sleipnir,_Geri,_Freki,_Huginn_and_Muninn_by_Fr%C3%B8lich.jpg#/media/File:Odin,_Sleipnir,_Geri,_Freki,_Huginn_and_Muninn_by_Frølich.jpg">
				<img src="https://upload.wikimedia.org/wikipedia/commons/3/31/Odin%2C_Sleipnir%2C_Geri%2C_Freki%2C_Huginn_and_Muninn_by_Fr%C3%B8lich.jpg" alt="Odin, Sleipnir, Geri, Freki, Huginn and Muninn by Frølich.jpg" width="200" height="200"/>
		</a>
		<br>
		By <a href="https://en.wikipedia.org/wiki/en:Lorenz_Fr%C3%B8lich">Lorenz Frølich</a></p>
	</a>
	<h2 align="center" > A C++ linter, free from the shackles of any build systems</h2>
</p>

Sleipnir is a C++ fast source code linter that strives to perform static analysis with minimal set-up overhead for the user.

## The tool relies on two open-source projects:

* [Cobra](https://github.com/nimble-code/Cobra)
* [Libtooling API from clang](https://clang.llvm.org/docs/LibTooling.html)

## Table of contents
- [Usage](#usage)
- [Steps to build](#step-to-build)
- [Testing](#testing)
- [Known issues](#known-issues)


## Steps to build

Sleipnir is tested on Mac with Homebrew and on Debian-based Linux distributions with the `apt` package manager.

### Install Dependencies
#### MacOS
```sh
$ brew install llvm@15 cmake
```

#### Debian-based distros
```sh
$ apt install cmake build-essential lsb-release wget software-properties-common gnupg
$ wget https://apt.llvm.org/llvm.sh
$ chmod +x ./llvm.sh
$ ./llvm.sh 15 all
```

### Build the tool
There is a single script to build, run, and test Sleipnir on both Mac and Debian based systems.

To build and run, set the `LLVM\_INSTALL\_DIR` variable with the LLVM installation directory. The default installation location by `brew` is _/usr/local/opt/llvm_ and it is _/usr/lib/llvm_ for `apt`.

```
$ LLVM_INSTALL_DIR=<path where apt/brew have install llvm> ./build.sh
```
You can view all the options for `build.sh` with the `--help` flag.
```
$ ./build.sh --help
usage: ./build.sh [--cc|--run <username/reponame>|--test|--test-checks|--clean-first]
usage: ./build.sh [--cc|--run <username/reponame>|--test|--test-checks|--clean-first]
	--cc                        build compile_commands.json in current dir
	--run <username/reponame>   build and run the LTA and cargo (git repo name should be username/reponame)
	--test                      build and run all tests
	--test-checks               build and run test for checks
	--clean-first               delete build dir and re-build
```

## Testing

We use [SCATR](https://github.com/deepsourcelabs/scatr) for testing checks. To run tests for checks, use `build.sh` with the `--test-checks` flag.
```
$ ./build.sh --test-checks
```

## Usage

We can use `build.sh --run` to run analysis on any accessible repository.
```
$ ./build.sh  --run username/reponame
```

For example,
```
$ ./build.sh --run goolge/googletest
```

## Known issues
* CMake version needs to be, `~3.22`.
* `llvm-15` is required and to be put in the primary folder if possible.
