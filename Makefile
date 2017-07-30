CMAKE_OPTS=-DCMAKE_BUILD_TYPE=Debug

all:
	@mkdir -p build
	@cd build; cmake ${CMAKE_OPTS} .. > /dev/null
	@make -C build --quiet
%:
	@mkdir -p build
	@cd build; cmake ${CMAKE_OPTS} .. > /dev/null
	@make -C build --quiet $@

.PHONY: release

release:
	@mkdir -p release
	@cd release; cmake .. > /dev/null
	@make -C release --quiet
