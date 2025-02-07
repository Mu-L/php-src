--TEST--
Bug #73072: Invalid path SNI_server_certs causes segfault
--EXTENSIONS--
openssl
--SKIPIF--
<?php
if (!function_exists("proc_open")) die("skip no proc_open");
?>
--FILE--
<?php
$serverCode = <<<'CODE'
    $flags = STREAM_SERVER_BIND|STREAM_SERVER_LISTEN;
    $ctx = stream_context_create(['ssl' => [
        'local_cert' => __DIR__ . '/domain1.pem',
        'SNI_server_certs' => [
            "domain1.com" => __DIR__ . "/sni_server_domain1.pem",
            "domain2.com" => __DIR__ . "/not_existing.pem",
        ]
    ]]);

    $server = stream_socket_server('tls://127.0.0.1:0', $errno, $errstr, $flags, $ctx);
    phpt_notify_server_start($server);

    @stream_socket_accept($server, 3);
    // if there is a segfault, this won't be called
    fwrite(STDERR, "done\n");
CODE;

$clientCode = <<<'CODE'
    $flags = STREAM_CLIENT_CONNECT;
    $ctxArr = [
        'cafile' => __DIR__ . '/sni_server_ca.pem',
        'capture_peer_cert' => true
    ];

    $ctxArr['peer_name'] = 'domain1.com';
    $ctx = stream_context_create(['ssl' => $ctxArr]);
    @stream_socket_client("tls://{{ ADDR }}", $errno, $errstr, 1, $flags, $ctx);
CODE;

include 'ServerClientTestCase.inc';
ServerClientTestCase::getInstance()->run($clientCode, $serverCode);
?>
--EXPECT--
done
