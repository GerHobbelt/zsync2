quicksync 0.6.4
===============

quicksync is a file transfer program. It allows you to download a file from a
remote web server, where you have a copy of an older version of the file on
your computer already. quicksync downloads only the new parts of the file. It
uses the same algorithm as rsync.

quicksync is a fork of zsync, to be used with [quickemu](https://github.com/wimpysworld/quickemu), and adds
an option to allow to output of the progress bar even if not started from a tty.
This is usefull for GUI developpers.

See the [main README](c/README) for more details.

NOTE : the documentation is from the original repo, and still refers to the binary
as `zsync`. Replace with `quicksync` accordingly. The `makezsync` binary is not built
by this fork.
