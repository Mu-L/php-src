--TEST--
Test function gzpassthru() by calling it with its expected arguments
--EXTENSIONS--
zlib
--FILE--
<?php

// note that gzpassthru is an alias to fpassthru. parameter checking tests will be
// the same as fpassthru

$f = __DIR__."/data/test.txt.gz";
$h = gzopen($f, 'r');
var_dump(gzpassthru($h));
var_dump(gzpassthru($h));
gzclose($h);

?>
--EXPECT--
When you're taught through feelings
Destiny flying high above
all I know is that you can realize it
Destiny who cares
as it turns around
and I know that it descends down on me
int(176)
int(0)
