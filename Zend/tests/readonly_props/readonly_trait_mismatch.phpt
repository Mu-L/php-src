--TEST--
Readonly mismatch of imported trait properties
--FILE--
<?php

trait T1 {
    public int $prop;
}
trait T2 {
    public readonly int $prop;
}
class C {
    use T1, T2;
}

?>
--EXPECTF--
Fatal error: T1 and T2 define the same property ($prop) in the composition of C. However, the definition differs and is considered incompatible. Class was composed in %s on line %d
