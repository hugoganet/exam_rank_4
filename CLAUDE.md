# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a **microshell** implementation - a 42 School exam project that creates a minimal shell program. The shell takes command line arguments and executes them with support for pipes (`|`) and sequential execution (`;`).

## Requirements (from subject.en.txt)

- **Expected file**: `microshell.c`
- **Allowed functions**: malloc, free, write, close, fork, waitpid, signal, kill, exit, chdir, execve, dup, dup2, pipe, strcmp, strncmp
- Must handle absolute/relative executable paths (no PATH resolution)
- Implement `|` (pipes) and `;` (sequential execution) like bash
- Built-in `cd` command with specific error handling
- Must handle hundreds of pipes with limited file descriptors (<30)

## Build Instructions

```bash
# Standard compilation (as per 42 norm)
gcc -Wall -Wextra -Werror microshell.c -o microshell
```

## Code Architecture

### Current Implementation Status
- `microshell.c` - Incomplete basic structure
- `microshell_1.c` - Complete working implementation (reference/study version)

### Core Components (`microshell_1.c`)

**Main Parser Loop**
- Iterates through argv, splitting on `;` and `|` separators
- Maintains `tmp_fd` for input redirection between commands
- Handles three execution modes: cd, sequential (`;`), and pipes (`|`)

**Built-in cd Command**
- Validates exactly 2 arguments (cd + path)
- Error messages: "error: cd: bad arguments" or "error: cd: cannot change directory to [path]"

**Command Execution (`ft_execute`)**
- Child process: sets up stdin, closes tmp_fd, calls execve
- Error handling: "error: cannot execute [executable]" on execve failure

**File Descriptor Management**
- Critical for handling hundreds of pipes with limited FDs
- Uses dup/dup2 for redirection, closes unused descriptors
- Parent waits for child completion and manages fd cleanup

## Error Handling Requirements

1. **cd errors**: Specific messages to stderr
2. **execve failure**: "error: cannot execute [path]" to stderr
3. **System call errors**: "error: fatal" to stderr + exit (except execve/chdir)
4. **File descriptor leaks**: Must not leak FDs (critical for pipe limits)

## Testing Examples

```bash
# Basic pipe and sequential execution
./microshell /bin/ls "|" /usr/bin/grep microshell ";" /bin/echo "i love my microshell"

# Built-in cd command
./microshell cd /tmp ";" /bin/pwd

# Error cases
./microshell cd  # Should show "bad arguments"
./microshell cd /nonexistent  # Should show "cannot change directory"
```

## Key Implementation Notes

- Environment must be passed to execve
- No PATH resolution - use absolute/relative paths only
- No wildcards or environment variable expansion needed
- Must handle edge cases with proper error messages to stderr