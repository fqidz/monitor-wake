CC = clang
ARGS = -xc\
	   -std=c99\
	   -O3\
	   -ferror-limit=0\
	   -Weverything\
	   -Werror\
	   -Wno-used-but-marked-unused\
	   -Wno-padded\
	   -Wno-declaration-after-statement\
	   -Wno-covered-switch-default\
	   -Wno-unsafe-buffer-usage\
	   -Wno-missing-prototypes\
	   -Wno-disabled-macro-expansion

OUTPUT_FILE = out/monitor-wake

monitor-wake: monitor-wake.c
	$(CC) $(ARGS) `pkg-config --cflags --libs dbus-1` monitor-wake.c -o $(OUTPUT_FILE)

clean:
	rm monitor-wake
