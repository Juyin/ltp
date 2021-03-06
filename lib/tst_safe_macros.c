// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <sys/ptrace.h>
#include "config.h"
#ifdef HAVE_SYS_FANOTIFY_H
# include <sys/fanotify.h>
#endif
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "lapi/setns.h"
#include "tst_safe_macros.h"
#include "lapi/personality.h"

int safe_setpgid(const char *file, const int lineno, pid_t pid, pid_t pgid)
{
	int rval;

	rval = setpgid(pid, pgid);
	if (rval) {
		tst_brk(TBROK | TERRNO,
		        "%s:%d: setpgid(%i, %i) failed",
			file, lineno, pid, pgid);
	}

	return rval;
}

pid_t safe_getpgid(const char *file, const int lineno, pid_t pid)
{
	pid_t pgid;

	pgid = getpgid(pid);
	if (pgid == -1) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: getpgid(%i) failed", file, lineno, pid);
	}

	return pgid;
}

int safe_personality(const char *filename, unsigned int lineno,
		    unsigned long persona)
{
	int prev_persona = personality(persona);

	if (prev_persona < 0) {
		tst_brk_(filename, lineno, TBROK | TERRNO,
			 "persona(%ld) failed", persona);
	}

	return prev_persona;
}

int safe_setregid(const char *file, const int lineno,
		  gid_t rgid, gid_t egid)
{
	int rval;

	rval = setregid(rgid, egid);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "setregid(%li, %li) failed",
			 (long)rgid, (long)egid);
	}

	return rval;
}

int safe_setreuid(const char *file, const int lineno,
		  uid_t ruid, uid_t euid)
{
	int rval;

	rval = setreuid(ruid, euid);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "setreuid(%li, %li) failed",
			 (long)ruid, (long)euid);
	}

	return rval;
}


int safe_sigaction(const char *file, const int lineno,
                   int signum, const struct sigaction *act,
                   struct sigaction *oldact)
{
	int rval;

	rval = sigaction(signum, act, oldact);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"sigaction(%s (%d), %p, %p) failed",
			tst_strsig(signum), signum, act, oldact);
	}

	return rval;
}

void safe_sigaddset(const char *file, const int lineno,
                    sigset_t *sigs, int signo)
{
	int rval;

	rval = sigaddset(sigs, signo);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
		         "sigaddset() %s (%i) failed",
			 tst_strsig(signo), signo);
	}
}

void safe_sigdelset(const char *file, const int lineno,
                    sigset_t *sigs, int signo)
{
	int rval;

	rval = sigdelset(sigs, signo);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
		         "sigdelset() %s (%i) failed",
			 tst_strsig(signo), signo);
	}
}

void safe_sigemptyset(const char *file, const int lineno,
                      sigset_t *sigs)
{
	int rval;

	rval = sigemptyset(sigs);
	if (rval == -1)
		tst_brk_(file, lineno, TBROK | TERRNO, "sigemptyset() failed");
}

void safe_sigfillset(const char *file, const int lineno,
		     sigset_t *sigs)
{
	int rval;

	rval = sigfillset(sigs);
	if (rval == -1)
		tst_brk_(file, lineno, TBROK | TERRNO, "sigfillset() failed");
}

static const char *strhow(int how)
{
	switch (how) {
	case SIG_BLOCK:
		return "SIG_BLOCK";
	case SIG_UNBLOCK:
		return "SIG_UNBLOCK";
	case SIG_SETMASK:
		return "SIG_SETMASK";
	default:
		return "???";
	}
}

void safe_sigprocmask(const char *file, const int lineno,
                      int how, sigset_t *set, sigset_t *oldset)
{
	int rval;

	rval = sigprocmask(how, set, oldset);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
		         "sigprocmask(%s, %p, %p)", strhow(how), set, oldset);
	}
}

void safe_sigwait(const char *file, const int lineno,
                  sigset_t *set, int *sig)
{
	int rval;

	rval = sigwait(set, sig);
	if (rval != 0) {
		errno = rval;
		tst_brk_(file, lineno, TBROK, "sigwait(%p, %p)", set, sig);
	}
}

struct group *safe_getgrnam(const char *file, const int lineno,
			    const char *name)
{
	struct group *rval;

	errno = 0;
	rval = getgrnam(name);
	if (rval == NULL) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getgrnam(%s) failed", name);
	}

	return rval;
}

struct group *safe_getgrnam_fallback(const char *file, const int lineno,
				     const char *name, const char *fallback)
{
	struct group *rval;

	errno = 0;
	rval = getgrnam(name);
	if (rval == NULL) {
		tst_res_(file, lineno, TINFO,
			 "getgrnam(%s) failed - try fallback %s",
			 name, fallback);
		rval = safe_getgrnam(file, lineno, fallback);
	}

	return rval;
}

struct group *safe_getgrgid(const char *file, const int lineno, gid_t gid)
{
	struct group *rval;

	errno = 0;
	rval = getgrgid(gid);
	if (rval == NULL) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"getgrgid(%li) failed", (long)gid);
	}

	return rval;
}

int safe_chroot(const char *file, const int lineno, const char *path)
{
	int rval;

	rval = chroot(path);
	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "chroot(%s) failed", path);
	}

	return rval;
}

void safe_unshare(const char *file, const int lineno, int flags)
{
	int res;

	res = unshare(flags);
	if (res == -1) {
		if (errno == EINVAL) {
			tst_brk_(file, lineno, TCONF | TERRNO,
				 "unshare(%d) unsupported", flags);
		} else {
			tst_brk_(file, lineno, TBROK | TERRNO,
				 "unshare(%d) failed", flags);
		}
	}
}

void safe_setns(const char *file, const int lineno, int fd, int nstype)
{
	int ret;

	ret = setns(fd, nstype);
	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "setns(%i, %i) failed",
		         fd, nstype);
	}
}

long tst_safe_ptrace(const char *file, const int lineno, int req, pid_t pid,
	void *addr, void *data)
{
	long ret;

	errno = 0;
	ret = ptrace(req, pid, addr, data);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO, "ptrace() failed");
	} else if (ret) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid ptrace() return value %ld", ret);
	}

	return ret;
}

int safe_pipe2(const char *file, const int lineno, int fildes[2], int flags)
{
	int ret;

	ret = pipe2(fildes, flags);
	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"pipe2({%d,%d}) failed with flag(%d)",
			fildes[0], fildes[1], flags);
	}

	return ret;
}
