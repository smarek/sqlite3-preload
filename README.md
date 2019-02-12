SHIM to debug libsqlite3.so.0 functions
	- sqlite3_bind_text
	- sqlite3_prepare_v2

This can be expanded upon


```
make
LD_PRELOAD=./sqlite3-preload.so /path/to/debuggee/application
```


Original project modified https://github.com/cventers/sqlite3-preload
