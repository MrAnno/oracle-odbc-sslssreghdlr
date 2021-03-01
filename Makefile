PKG_CONFIG_ODBC_NAME ?= odbc

CPPFLAGS += -D_POSIX_C_SOURCE=200112L
CFLAGS += -std=c99
LDFLAGS += -pthread

CFLAGS += $(shell pkg-config --cflags ${PKG_CONFIG_ODBC_NAME})
LDFLAGS += $(shell pkg-config --libs-only-L ${PKG_CONFIG_ODBC_NAME})
LDFLAGS += $(shell pkg-config --libs-only-other ${PKG_CONFIG_ODBC_NAME})
LDLIBS += $(shell pkg-config --libs-only-l ${PKG_CONFIG_ODBC_NAME})

.PHONY: build
build: oracle-odbc-sslssreghdlr

.PHONY: clean
clean:
	-rm oracle-odbc-sslssreghdlr
