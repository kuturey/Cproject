# MiniGit - Simplified Version Control System

A lightweight Git-like VCS implemented in C with persistent storage, branching, and structural sharing.

## Features
- ✅ Persistent storage between runs
- ✅ Commit history with parent links
- ✅ Branch creation, switching, deletion
- ✅ Staging area (add/commit workflow)
- ✅ Object storage with deduplication
- ✅ Structural sharing (trees)

## Build
```bash
gcc -Iinclude src/core/*.c src/commands/*.c src/main.c -o bin/minigit.exe