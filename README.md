# Mini-Linux-Shell (MY_SHELL)

> A minimal Unix/Linux command-line shell written from scratch in C — read, parse, fork, exec, wait.

## Overview

**MY_SHELL** is a small interactive command interpreter that reproduces the core
of a Unix shell using low-level POSIX system calls. It reads a line from the
terminal, tokenizes it, and either runs the request as one of its **built-in
commands** or spawns a child process to execute a matching **external command**
program shipped in the `cmds/` directory.

The project is intentionally focused on the fundamentals of systems programming:
process creation (`fork`), program loading (`execv`), synchronization
(`waitpid`), file-descriptor management (`dup`/`dup2`), and manual string
tokenization — with no external libraries beyond the C standard library and the
POSIX system-call interface.

The codebase has since been **hardened for correctness and memory safety** (see
[Robustness & Hardening](#robustness--hardening)): unbounded `strcpy`/`strcat`
calls were replaced with bounded `snprintf`, the `ls -l` permission-bit
rendering and EOF handling were fixed, `getpwuid`/`getgrgid` lookups are
`NULL`-guarded, and the per-command heap allocations are freed each iteration.

## Features

Backed by the source in `Mini-Shell/miniShell.c` and `Mini-Shell/cmds/`:

- **Interactive REPL** with a `MY_SHELL> ` prompt (`shell_loop`).
- **Line reading** from the terminal, character by character, into a
  dynamically grown buffer (`read_command_line`).
- **Whitespace tokenization** of the command line into an argument vector
  (`split_command_line`, backed by `strtok`).
- **Built-in commands (5)**, dispatched via an array of function pointers:
  - `cd <dir>` — change the current working directory (`chdir`).
  - `pwd` — print the tracked working directory.
  - `echo <args...>` — print the supplied arguments.
  - `help` — list the available commands.
  - `exit` — terminate the shell.
- **External commands** launched as separate processes via `fork` + `execv`,
  resolved from the shell's bundled `cmds/` directory. Included programs:
  - `ls [-a] [-l] [-i] [dir ...]` — directory listing with support for the
    combined `-a` (show hidden), `-l` (long format: file type, permission bits,
    link count, owner, group, size, and last-modified time), and `-i` (inode
    number) options, with entries sorted case-insensitively. Flags may be
    grouped (e.g. `ls -al`), and multiple directories may be listed at once.
  - `cat <file ...>` — print the contents of one or more files.
  - `mkdir <dir ...>` — create one or more directories (`mkdir`, mode `0755`).
  - `rmdir <dir ...>` — remove one or more (empty) directories.
  - `clear` — clear the terminal screen via ANSI escape codes.
- **Environment tracking** through the `PWD` and `PATH` globals; `PWD` is kept in
  sync on every `cd`, and `PATH` points external-command lookups at `cmds/`.
- **Terminal state hygiene**: `stdin`, `stdout`, and `stderr` are saved with
  `dup` before each command and restored with `dup2` afterward.
- **Error reporting** through `perror`/`fprintf(stderr, ...)` prefixed with
  `MY_SHELL`.

> Note on scope: this is a teaching-oriented shell. It does **not** implement
> pipes (`|`), I/O redirection (`>`, `<`, `>>`), background jobs (`&`), quoting,
> globbing, or signal handling. See [Possible Improvements](#possible-improvements).

## Tech Stack

- **Language:** C (C99-compatible; standard C library only).
- **Platform:** Unix/Linux (developed and tested on Ubuntu 20.04).
- **Build tool:** `gcc` (invoked directly; no Makefile).
- **POSIX / system APIs used:**
  - Process control: `fork`, `execv`, `waitpid`, `exit`, `pid_t`
    (`unistd.h`, `sys/wait.h`).
  - File descriptors & I/O: `dup`, `dup2`, `open`, `read`, `write`, `close`
    (`unistd.h`, `fcntl.h`).
  - Filesystem: `chdir`, `getcwd`, `mkdir`, `rmdir`, `opendir`, `readdir`,
    `stat` (`unistd.h`, `sys/stat.h`, `dirent.h`).
  - Metadata resolution: `getpwuid`, `getgrgid`, `localtime`, `strftime`
    (`pwd.h`, `grp.h`, `time.h`).
  - Strings & memory: `strtok`, `strcmp`, `snprintf` (bounded formatting),
    `malloc`, `realloc`, `free` (`string.h`, `stdlib.h`).

## How It Works

### The shell loop

`main()` initializes the environment: it reads the current directory into `PWD`
via `getcwd`, then builds `PATH` as `<cwd>/cmds/` so external-command lookups
target the bundled programs. It then calls `shell_loop()`.

`shell_loop()` prints the `help` banner once, then repeatedly:

1. Prints the prompt `MY_SHELL> `.
2. Reads a line with `read_command_line()`.
3. Tokenizes it with `split_command_line()`.
4. Dispatches it with `shell_execute()`.

The loop continues while the returned status is non-zero; `exit` returns `0`,
which ends the loop.

### Reading & parsing

- `read_command_line()` allocates a 1 KB buffer and reads characters with
  `getchar()` until it hits `'\n'` or `EOF`, growing the buffer with `realloc`
  when needed.
- `split_command_line()` splits the raw line on spaces using `strtok()`, filling
  a `char **` argument vector that is `NULL`-terminated — the exact shape
  `execv()` expects.

### The process model

`shell_execute()` decides how to run each command:

- **Built-in path:** it linearly compares `args[0]` against the `builtin[]`
  name table (`cd`, `exit`, `help`, `pwd`, `echo`). On a match it invokes the
  paired handler through the `builtin_function[]` array of function pointers —
  no `switch` statement required.
- **External path:** for anything else it calls `start_process(args)`, which:
  1. Calls `fork()` to create a child.
  2. In the **child**, builds the absolute program path (`PATH + args[0]`) and
     replaces its image with `execv(cmd_dir, args)`; on failure it reports via
     `perror` and `exit(EXIT_FAILURE)`.
  3. In the **parent**, calls `waitpid(pid, &status, WUNTRACED)` in a loop until
     the child has exited (`WIFEXITED`) or was signaled (`WIFSIGNALED`).

Before dispatch, `shell_execute()` snapshots `stdin`/`stdout`/`stderr` with
`dup(0/1/2)` and restores them with `dup2` after the command returns, keeping
the shell's terminal state clean between commands.

### External commands as standalone programs

Each external command lives in `Mini-Shell/cmds/` as its own `.c` file compiled
to a binary of the same name. For example, `ls` is `cmds/ls.c` compiled to
`cmds/ls`. The shell finds them by name in that directory — they are ordinary
programs the shell `execv`s, exactly like the real Unix model.

## Getting Started

### Prerequisites

- A Unix/Linux environment (e.g., Ubuntu).
- `gcc` (or any C99 compiler).

### Build

There is no Makefile; compile the shell and each external command directly.

```bash
cd Mini-Shell

# Build the shell itself
gcc miniShell.c -o miniShell

# Build the external commands into the cmds/ directory
gcc cmds/ls.c    -o cmds/ls
gcc cmds/cat.c   -o cmds/cat
gcc cmds/mkdir.c -o cmds/mkdir
gcc cmds/rmdir.c -o cmds/rmdir
gcc cmds/clear.c -o cmds/clear
```

> External commands are resolved relative to the directory the shell is started
> from (`<cwd>/cmds/`), so run the shell from the `Mini-Shell` directory.

### Run

```bash
cd Mini-Shell
./miniShell
```

You will see the `help` banner followed by the `MY_SHELL> ` prompt.

## Usage / Examples

```text
MY_SHELL> pwd
/home/user/Mini-Shell

MY_SHELL> cd cmds
MY_SHELL> pwd
/home/user/Mini-Shell/cmds

MY_SHELL> echo hello mini shell
hello mini shell

MY_SHELL> mkdir demo
MY_SHELL> ls
...
demo
...

MY_SHELL> ls -al
d rwx r-x r-x   5 user   user      4096  Jul 04 12:00   .
...

MY_SHELL> ls -i
...

MY_SHELL> cat test.txt
Hi ^-^
This is a test file to display

MY_SHELL> rmdir demo

MY_SHELL> clear

MY_SHELL> help

MY_SHELL> exit
```

## Project Structure

```text
Mini-Linux-Shell-master/
├── README.md              # This file
└── Mini-Shell/
    ├── miniShell.c        # Shell core: REPL, tokenizer, dispatcher, fork/exec/wait, built-ins
    ├── Read_ME_FIRST.txt   # Original project notes
    ├── test.txt           # Sample file for trying `cat`
    └── cmds/              # External commands (one program per file)
        ├── ls.c           # Directory listing with -a / -l / -i options
        ├── cat.c          # Print file contents
        ├── mkdir.c        # Create directories
        ├── rmdir.c        # Remove directories
        └── clear.c        # Clear the terminal (ANSI escape)
```

## Robustness & Hardening

The original implementation has been reworked for correctness and memory safety.
Concretely:

- **Buffer-overflow prevention:** unbounded `strcpy`/`strcat` calls (used to
  build the command path and the `PATH` string) were replaced with bounded
  `snprintf`, so oversized input can no longer overrun fixed-size stack buffers.
- **Correct `ls -l` permissions:** the long-listing permission field now renders
  each user/group/other bit (`r`/`w`/`x` vs `-`) from `st_mode` correctly,
  matching real `ls`.
- **EOF / `getchar` handling:** `read_command_line()` reads with an `int` and
  stops on both `'\n'` and `EOF`, always NUL-terminating the buffer, so a
  Ctrl-D / piped EOF no longer misbehaves.
- **`NULL`-guarded metadata lookups:** `getpwuid`/`getgrgid` can return `NULL`
  for orphaned UIDs/GIDs; the long listing now checks for that and falls back to
  the numeric id instead of dereferencing a null pointer.
- **No per-command leaks:** the argument vector and command-line buffer returned
  by `split_command_line()` / `read_command_line()` are `free`d on every loop
  iteration (including the empty-input fast path), so a long session no longer
  leaks heap memory.

## Possible Improvements

- Add **I/O redirection** (`>`, `<`, `>>`) using the already-imported
  `open`/`dup2` primitives.
- Add **pipelines** (`|`) with `pipe()` + `fork` + `dup2`.
- Support **background execution** (`&`) and basic **job control** with signal
  handling (`SIGINT`, `SIGCHLD`).
- Fall back to the system `$PATH` (`execvp`) when a command is not found in
  `cmds/`.
- Add quoting and escape handling in the tokenizer.

## Author

**Aviral Kumar Singh** — [github.com/Apilex100](https://github.com/Apilex100)

*Originally developed as a collaborative project (Aviral, Avishek, Gulshan, Varun).*
