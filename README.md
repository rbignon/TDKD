Trou du cul (Asshole)
=====================

This is an ooold project I wrote in 2002-2004. The server runs on Linux and is written in C, and the client runs on Windows and is written in pascal.

Using it
--------

Just launch:

```$ make```

Then, you have to create two directories:

```$ mkdir database logs```

Finally, run the daemon with:

```$ ./troudukd```

Where is the client?
--------------------

The client sources are available on https://github.com/rbignon/trouduk, but there is an .exe inside this repository. Just change the hostname of course, as inter-system.net and coderz.info domain names aren't available anymore.

I'm not sure this works on a real Windows, but it is ok with wine.

If you want to use a public server, insert "symlink.me:5123".
