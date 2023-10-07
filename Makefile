# Alternative GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug_x64
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug_x64)
  raylib_config = debug_x64
  client_config = debug_x64
  networking_config = debug_x64
  server_config = debug_x64

else ifeq ($(config),debug_x86)
  raylib_config = debug_x86
  client_config = debug_x86
  networking_config = debug_x86
  server_config = debug_x86

else ifeq ($(config),debug_arm64)
  raylib_config = debug_arm64
  client_config = debug_arm64
  networking_config = debug_arm64
  server_config = debug_arm64

else ifeq ($(config),release_x64)
  raylib_config = release_x64
  client_config = release_x64
  networking_config = release_x64
  server_config = release_x64

else ifeq ($(config),release_x86)
  raylib_config = release_x86
  client_config = release_x86
  networking_config = release_x86
  server_config = release_x86

else ifeq ($(config),release_arm64)
  raylib_config = release_arm64
  client_config = release_arm64
  networking_config = release_arm64
  server_config = release_arm64

else
  $(error "invalid configuration $(config)")
endif

PROJECTS := raylib client networking server

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

raylib:
ifneq (,$(raylib_config))
	@echo "==== Building raylib ($(raylib_config)) ===="
	@${MAKE} --no-print-directory -C _build -f raylib.make config=$(raylib_config)
endif

client: raylib networking
ifneq (,$(client_config))
	@echo "==== Building client ($(client_config)) ===="
	@${MAKE} --no-print-directory -C _build -f client.make config=$(client_config)
endif

networking:
ifneq (,$(networking_config))
	@echo "==== Building networking ($(networking_config)) ===="
	@${MAKE} --no-print-directory -C _build -f networking.make config=$(networking_config)
endif

server: networking
ifneq (,$(server_config))
	@echo "==== Building server ($(server_config)) ===="
	@${MAKE} --no-print-directory -C _build -f server.make config=$(server_config)
endif

clean:
	@${MAKE} --no-print-directory -C _build -f raylib.make clean
	@${MAKE} --no-print-directory -C _build -f client.make clean
	@${MAKE} --no-print-directory -C _build -f networking.make clean
	@${MAKE} --no-print-directory -C _build -f server.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug_x64"
	@echo "  debug_x86"
	@echo "  debug_arm64"
	@echo "  release_x64"
	@echo "  release_x86"
	@echo "  release_arm64"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   raylib"
	@echo "   client"
	@echo "   networking"
	@echo "   server"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"