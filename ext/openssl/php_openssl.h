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
   | Authors: Stig Venaas <venaas@php.net>                                |
   |          Wez Furlong <wez@thebrainroom.com                           |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_OPENSSL_H
#define PHP_OPENSSL_H

#ifdef HAVE_OPENSSL_EXT
extern zend_module_entry openssl_module_entry;
#define phpext_openssl_ptr &openssl_module_entry

#include "php_version.h"
#define PHP_OPENSSL_VERSION PHP_VERSION

#include <openssl/opensslv.h>
/* OpenSSL version check */
#if OPENSSL_VERSION_NUMBER < 0x30000000L
/* This includes LibreSSL that defines version 0x20000000L */
#define PHP_OPENSSL_API_VERSION 0x10100
#elif OPENSSL_VERSION_NUMBER < 0x30200000L
#define PHP_OPENSSL_API_VERSION 0x30000
#else
#define PHP_OPENSSL_API_VERSION 0x30200
#endif

#define OPENSSL_RAW_DATA 1
#define OPENSSL_ZERO_PADDING 2
#define OPENSSL_DONT_ZERO_PAD_KEY 4

#define OPENSSL_ERROR_X509_PRIVATE_KEY_VALUES_MISMATCH 0x0B080074

/* Used for client-initiated handshake renegotiation DoS protection*/
#define OPENSSL_DEFAULT_RENEG_LIMIT 2
#define OPENSSL_DEFAULT_RENEG_WINDOW 300
#define OPENSSL_DEFAULT_STREAM_VERIFY_DEPTH 9
#define OPENSSL_DEFAULT_STREAM_CIPHERS "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:" \
	"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:" \
	"DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:" \
	"ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:" \
	"ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:" \
	"DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:" \
	"AES256-GCM-SHA384:AES128:AES256:HIGH:!SSLv2:!aNULL:!eNULL:!EXPORT:!DES:!MD5:!RC4:!ADH"

#include <openssl/err.h>

#ifdef PHP_WIN32
#	define PHP_OPENSSL_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_OPENSSL_API __attribute__((visibility("default")))
#else
#	define PHP_OPENSSL_API
#endif

struct php_openssl_errors {
	int buffer[ERR_NUM_ERRORS];
	int top;
	int bottom;
};

struct php_openssl_libctx {
#if PHP_OPENSSL_API_VERSION >= 0x30000
	OSSL_LIB_CTX *libctx;
	OSSL_LIB_CTX *default_libctx;
	OSSL_LIB_CTX *custom_libctx;
#endif
	char *propq;
};

ZEND_BEGIN_MODULE_GLOBALS(openssl)
	struct php_openssl_errors *errors;
	struct php_openssl_errors *errors_mark;
	struct php_openssl_libctx ctx;
ZEND_END_MODULE_GLOBALS(openssl)

#define PHP_OPENSSL_LIBCTX OPENSSL_G(ctx).libctx
#define PHP_OPENSSL_PROPQ OPENSSL_G(ctx).propq

#define OPENSSL_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openssl, v)

#if defined(ZTS) && defined(COMPILE_DL_OPENSSL)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

php_stream_transport_factory_func php_openssl_ssl_socket_factory;

void php_openssl_store_errors(void);
void php_openssl_errors_set_mark(void);
void php_openssl_errors_restore_mark(void);

/* openssl file path extra */
bool php_openssl_check_path_ex(
		const char *file_path, size_t file_path_len, char *real_path, uint32_t arg_num,
		bool contains_file_protocol, bool is_from_array, const char *option_name);

/* openssl file path check */
static inline bool php_openssl_check_path(
		const char *file_path, size_t file_path_len, char *real_path, uint32_t arg_num)
{
	return php_openssl_check_path_ex(
			file_path, file_path_len, real_path, arg_num, false, false, NULL);
}

/* openssl file path extra check with zend string */
static inline bool php_openssl_check_path_str_ex(
		zend_string *file_path, char *real_path, uint32_t arg_num,
		bool contains_file_protocol, bool is_from_array, const char *option_name)
{
	return php_openssl_check_path_ex(
			ZSTR_VAL(file_path), ZSTR_LEN(file_path), real_path, arg_num, contains_file_protocol,
			is_from_array, option_name);
}

/* openssl file path check with zend string */
static inline bool php_openssl_check_path_str(
		zend_string *file_path, char *real_path, uint32_t arg_num)
{
	return php_openssl_check_path_str_ex(file_path, real_path, arg_num, true, false, NULL);
}

PHP_OPENSSL_API zend_long php_openssl_cipher_iv_length(const char *method);
PHP_OPENSSL_API zend_long php_openssl_cipher_key_length(const char *method);
PHP_OPENSSL_API zend_string* php_openssl_random_pseudo_bytes(zend_long length);
PHP_OPENSSL_API zend_string* php_openssl_encrypt(
	const char *data, size_t data_len,
	const char *method, size_t method_len,
	const char *password, size_t password_len,
	zend_long options,
	const char *iv, size_t iv_len,
	zval *tag, zend_long tag_len,
	const char *aad, size_t aad_len);
PHP_OPENSSL_API zend_string* php_openssl_decrypt(
	const char *data, size_t data_len,
	const char *method, size_t method_len,
	const char *password, size_t password_len,
	zend_long options,
	const char *iv, size_t iv_len,
	const char *tag, zend_long tag_len,
	const char *aad, size_t aad_len);

/* OpenSSLCertificate class */

#include <openssl/x509.h>

typedef struct _php_openssl_certificate_object {
	X509 *x509;
	zend_object std;
} php_openssl_certificate_object;

extern zend_class_entry *php_openssl_certificate_ce;

static inline php_openssl_certificate_object *php_openssl_certificate_from_obj(zend_object *obj) {
	return (php_openssl_certificate_object *)((char *)(obj) - XtOffsetOf(php_openssl_certificate_object, std));
}

#define Z_OPENSSL_CERTIFICATE_P(zv) php_openssl_certificate_from_obj(Z_OBJ_P(zv))

bool php_openssl_is_certificate_ce(zval *val);

/* OpenSSLCertificateSigningRequest class */

typedef struct _php_openssl_x509_request_object {
	X509_REQ *csr;
	zend_object std;
} php_openssl_request_object;

static inline php_openssl_request_object *php_openssl_request_from_obj(zend_object *obj) {
	return (php_openssl_request_object *)((char *)(obj) - XtOffsetOf(php_openssl_request_object, std));
}

#define Z_OPENSSL_REQUEST_P(zv) php_openssl_request_from_obj(Z_OBJ_P(zv))

bool php_openssl_is_request_ce(zval *val);

/* OpenSSLAsymmetricKey class */

typedef struct _php_openssl_pkey_object {
	EVP_PKEY *pkey;
	bool is_private;
	zend_object std;
} php_openssl_pkey_object;

static inline php_openssl_pkey_object *php_openssl_pkey_from_obj(zend_object *obj) {
	return (php_openssl_pkey_object *)((char *)(obj) - XtOffsetOf(php_openssl_pkey_object, std));
}

#define Z_OPENSSL_PKEY_P(zv) php_openssl_pkey_from_obj(Z_OBJ_P(zv))

bool php_openssl_is_pkey_ce(zval *val);
void php_openssl_pkey_object_init(zval *zv, EVP_PKEY *pkey, bool is_private);

#if defined(HAVE_OPENSSL_ARGON2)

/**
 * MEMLIMIT is normalized to KB even though sodium uses Bytes in order to
 * present a consistent user-facing API.
 *
 * When updating these values, synchronize ext/standard/php_password.h values.
 */
#if defined(PHP_PASSWORD_ARGON2_MEMORY_COST)
#define PHP_OPENSSL_PWHASH_MEMLIMIT PHP_PASSWORD_ARGON2_MEMORY_COST
#else
#define PHP_OPENSSL_PWHASH_MEMLIMIT (64 << 10)
#endif
#if defined(PHP_PASSWORD_ARGON2_TIME_COST)
#define PHP_OPENSSL_PWHASH_ITERLIMIT PHP_PASSWORD_ARGON2_TIME_COST
#else
#define PHP_OPENSSL_PWHASH_ITERLIMIT 4
#endif
#if defined(PHP_PASSWORD_ARGON2_THREADS)
#define PHP_OPENSSL_PWHASH_THREADS PHP_PASSWORD_ARGON2_THREADS
#else
#define PHP_OPENSSL_PWHASH_THREADS 1
#endif

#endif

PHP_MINIT_FUNCTION(openssl);
PHP_MSHUTDOWN_FUNCTION(openssl);
PHP_MINFO_FUNCTION(openssl);
PHP_GINIT_FUNCTION(openssl);
PHP_GSHUTDOWN_FUNCTION(openssl);
#if defined(HAVE_OPENSSL_ARGON2)
PHP_MINIT_FUNCTION(openssl_pwhash);
#endif

#ifdef PHP_WIN32
#define PHP_OPENSSL_BIO_MODE_R(flags) (((flags) & PKCS7_BINARY) ? "rb" : "r")
#define PHP_OPENSSL_BIO_MODE_W(flags) (((flags) & PKCS7_BINARY) ? "wb" : "w")
#else
#define PHP_OPENSSL_BIO_MODE_R(flags) "r"
#define PHP_OPENSSL_BIO_MODE_W(flags) "w"
#endif

#else

#define phpext_openssl_ptr NULL

#endif


#endif
