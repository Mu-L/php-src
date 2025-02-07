/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author:                                                              |
   +----------------------------------------------------------------------+
 */

#include "php.h"

#if defined(HAVE_SYMLINK) || defined(PHP_WIN32)

#ifdef PHP_WIN32
#include <WinBase.h>
#endif

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef PHP_WIN32
#include <sys/stat.h>
#endif
#include <string.h>
#include <errno.h>

#include "php_string.h"

#ifndef VOLUME_NAME_NT
#define VOLUME_NAME_NT 0x2
#endif

#ifndef VOLUME_NAME_DOS
#define VOLUME_NAME_DOS 0x0
#endif

/* {{{ Return the target of a symbolic link */
PHP_FUNCTION(readlink)
{
	char *link;
	size_t link_len;
	char buff[MAXPATHLEN];
	ssize_t ret;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_PATH(link, link_len)
	ZEND_PARSE_PARAMETERS_END();

	if (php_check_open_basedir(link)) {
		RETURN_FALSE;
	}

	ret = php_sys_readlink(link, buff, MAXPATHLEN-1);

	if (ret == -1) {
#ifdef PHP_WIN32
		php_error_docref(NULL, E_WARNING, "readlink failed to read the symbolic link (%s), error %ld", link, GetLastError());
#else
		php_error_docref(NULL, E_WARNING, "%s", strerror(errno));
#endif
		RETURN_FALSE;
	}
	/* Append NULL to the end of the string */
	buff[ret] = '\0';

	RETURN_STRINGL(buff, ret);
}
/* }}} */

/* {{{ Returns the st_dev field of the UNIX C stat structure describing the link */
PHP_FUNCTION(linkinfo)
{
	char *link;
	char *dirname;
	size_t link_len;
	zend_stat_t sb = {0};
	int ret;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_PATH(link, link_len)
	ZEND_PARSE_PARAMETERS_END();

	// TODO Check for empty string
	dirname = estrndup(link, link_len);
	zend_dirname(dirname, link_len);

	if (php_check_open_basedir(dirname)) {
		efree(dirname);
		RETURN_FALSE;
	}

	ret = VCWD_LSTAT(link, &sb);
	if (ret == -1) {
		php_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		efree(dirname);
		RETURN_LONG(Z_L(-1));
	}

	efree(dirname);
	RETURN_LONG((zend_long) sb.st_dev);
}
/* }}} */

/* {{{ Create a symbolic link */
PHP_FUNCTION(symlink)
{
	char *topath, *frompath;
	size_t topath_len, frompath_len;
	int ret;
	char source_p[MAXPATHLEN];
	char dest_p[MAXPATHLEN];
	char dirname[MAXPATHLEN];
	size_t len;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_PATH(topath, topath_len)
		Z_PARAM_PATH(frompath, frompath_len)
	ZEND_PARSE_PARAMETERS_END();

	if (!expand_filepath(frompath, source_p)) {
		php_error_docref(NULL, E_WARNING, "No such file or directory");
		RETURN_FALSE;
	}

	memcpy(dirname, source_p, sizeof(source_p));
	len = zend_dirname(dirname, strlen(dirname));

	if (!expand_filepath_ex(topath, dest_p, dirname, len)) {
		php_error_docref(NULL, E_WARNING, "No such file or directory");
		RETURN_FALSE;
	}

	if (php_stream_locate_url_wrapper(source_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) ||
		php_stream_locate_url_wrapper(dest_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) )
	{
		php_error_docref(NULL, E_WARNING, "Unable to symlink to a URL");
		RETURN_FALSE;
	}

	if (php_check_open_basedir(dest_p)) {
		RETURN_FALSE;
	}

	if (php_check_open_basedir(source_p)) {
		RETURN_FALSE;
	}

	/* For the source, an expanded path must be used (in ZTS an other thread could have changed the CWD).
	 * For the target the exact string given by the user must be used, relative or not, existing or not.
	 * The target is relative to the link itself, not to the CWD. */
	ret = php_sys_symlink(topath, source_p);

	if (ret == -1) {
		php_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Create a hard link */
PHP_FUNCTION(link)
{
	char *topath, *frompath;
	size_t topath_len, frompath_len;
	int ret;
	char source_p[MAXPATHLEN];
	char dest_p[MAXPATHLEN];

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_PATH(topath, topath_len)
		Z_PARAM_PATH(frompath, frompath_len)
	ZEND_PARSE_PARAMETERS_END();

	if (!expand_filepath(frompath, source_p) || !expand_filepath(topath, dest_p)) {
		php_error_docref(NULL, E_WARNING, "No such file or directory");
		RETURN_FALSE;
	}

	if (php_stream_locate_url_wrapper(source_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) ||
		php_stream_locate_url_wrapper(dest_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) )
	{
		php_error_docref(NULL, E_WARNING, "Unable to link to a URL");
		RETURN_FALSE;
	}

	if (php_check_open_basedir(dest_p)) {
		RETURN_FALSE;
	}

	if (php_check_open_basedir(source_p)) {
		RETURN_FALSE;
	}

#ifndef ZTS
	ret = php_sys_link(topath, frompath);
#else
	ret = php_sys_link(dest_p, source_p);
#endif
	if (ret == -1) {
		php_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

#endif
