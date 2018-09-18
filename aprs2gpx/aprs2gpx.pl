#!/usr/bin/perl
while (<STDIN>) {
        print if (/^201[56]-[0-1][0-9]-[0-3][0-9]
                   \s[0-2][0-9]:[0-5][0-9]:[0-5][0-9]\s(UTC|EDT):\s
                   (N1YIP|W1YA|KB1YOF|N1ME)-11>(APRS|APZUME),
                   ((\w|-|\*)+,)+qA[\w],((\w|-|\*)+):\/(\d+)(h|z)
                   (\d+)\.(\d+)[NS]\/(\d+)\.(\d+)[EW]O
                   (\d+)\/(\d+)\/A=(\d+)
                   ((\/\w+=-?\d+\.*\d*)|(!w[!-~][!-~]!))+
                   \s((147\.57\sup\s446\.10\sdown$)|(Umaine\sHAB$))*/x);
        next;
}


