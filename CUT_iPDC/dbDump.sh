#!/bin/sh

mysqlimport --local --fields-terminated-by=\, --fields-enclosed-by=\\\" -uroot -proot iPDC LOG_TIME_INFO.txt
