# PS4JB2 (v3) My own Fork

This repository contains the source files for the 6.72 version of Sleirsgovey's ps4jb2/672/src repository. The intention of this repository is to allow me to make my own changes, modifications, attempted improvements, etc. Essentially, it serves as a custom fork of the original repository, which I will maintain.

## File (inside src/) Information

The `netcat.c` file loads 65536 bytes at the address stored in the JS variable `mira_blob` into RWX memory and jumps to it. At this point, only the minimal patches (amd64_syscall, mmap, mprotect, kexec) are applied, which means the process is still "sandboxed." Normally, `mira_blob` contains [MiraLoader](https://github.com/a0zhar/PS4PayloadLoader/).

In the background, `mira_blob_2_len` bytes at `mira_blob_2` are sent to `127.0.0.1:9021` in a separate thread. If `mira_blob` contains [MiraLoader](https://github.com/a0zhar/PS4PayloadLoader/), it will run in the same way but with the full patch set applied and the device already jailbroken.
