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
	@cd release; cmake -D CMAKE_INSTALL_PREFIX=/usr .. > /dev/null
	@make -C release --quiet

install-release: release
	@make -C release --quiet install

clean-release:
	@rm -rf release
