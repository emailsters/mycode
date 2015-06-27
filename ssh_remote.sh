#!/usr/bin/expect -f

if { $argc < 3 } {
    puts stderr "Usage: $argv0 ipaddress account password command[script]"
    exit
}

set IPADDR [lindex $argv 0]
set ACCOUNT [lindex $argv 1]
set OLD_PW [lindex $argv 2]
set CMD [lindex $argv 3]

set timeout 30

stty -echo

spawn ssh $IPADDR -l $ACCOUNT $CMD
expect {
    "*connecting (yes/no)*" {
        send "yes"
        exp_continue
    }
    "*assword:*"  {
        send "$OLD_PW\r"
        exp_continue
    } "*Last login:*" {
        interact
        exit 0
    } timeout {
        send_user "connection to $IPADDR timeout!\n"
        exit 1
    } "*incorrect*" {
        send_user "password incorrect!\n"
        exit 2
    } "*Permission*" {
        send_user "password Error!\n"
        exit 2
    } eof {
        exit 3
    }
}
