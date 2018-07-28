# Piping for `msmtp`

`msmtp` is a simple programme that can pretend to be `sendmail` on your system.
Once configured it will take an input file (or stdin) and use the configured
smtp server to send an email to the appropriate recipient.

## Configuring `msmtp`

This is repeated in many, many places but for completeness here we go...

    account bengreeneu
    host localhost
    from benjamin@example.com
    port 25
    auth on
    user benjamin@example.com
    password mypasswordissecret
    logfile ~/.msmtp.log
    tls on
    tls_starttls on
    tls_certcheck off

Adjust to taste ;-).

## Spawn the `msmtp` process and pipe to it

To make this nice and easy, I have created a
[github repository](https://github.com/drcrane/pipetosubprocess)
that contains the example project and a `Makefile`.

The basic steps in this programme are:

* Create two pipes, one for `STDIN` and one for `STDOUT` they are named from
the perspective of the parent process (`wpipefd` is write from parent to
child and `rpipefd` is read from child to parent).
* Re-assign the file descriptors in the forked process so that `STDIN_FILENO`
and `STDOUT_FILENO` are connected to the pipe.
* Replace the current executable image in the forked process with the `execv`
system call. The new process will have the normal 3 file descriptors:
 * stdin (`STDIN_FILENO` / 0) connected to `wpipefd`
 * stdout (`STDOUT_FILENO` / 1) connected to `rpipefd`
 * stderr (`STDERR_FILENO` / 2) this is untouched so will be the same as the
parent process... good for debugging!
* Write the email to the pipe, the Linux kernel will buffer the bytes written
unless you have some funny kernel configuration.
* Read the output from `rpipefd` until the process closes the other side of
the `rpipefd` (ie. the child process closes stdout or the process terminates).

## Compiling and Running the Programme

To run this programme successfully msmtp must be installed on your system.
For simplicity it is hard coded in `emailer.c` to `/usr/bin/msmtp` change if
necessary.

    make testsrc/emailer_test
    testsrc/emailer_test destination@email.address sometoken

Should be it, please let me know if you have any problems, I will update this
repository with worthwhile feedback.

## Further Work

It might be cool to use the techniques learnt here to extend the little shell
[described here](https://brennan.io/2015/01/16/write-a-shell-in-c/) so it can
pipe stuff like a more fully featured shell.


