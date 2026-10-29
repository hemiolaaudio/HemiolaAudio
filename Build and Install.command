#!/bin/bash
# Hemiola Audio: build, test and install into VCV Rack (Mac).
# Double-click this file. It writes a full log to "Build log.txt" in the Claude VCV folder.

cd "$(dirname "$0")" || exit 1
HERE="$(pwd)"
LOG="$HERE/../Build log.txt"
SDK="$HERE/../Rack-SDK"
PLUGINS="$HOME/Library/Application Support/Rack2"

exec > >(tee "$LOG") 2>&1

echo "Hemiola Audio build, $(date)"
echo "Folder: $HERE"
echo

finish() {
	echo
	echo "=============================================="
	echo "$1"
	echo "=============================================="
	echo "A full log was saved as: Build log.txt (in the Claude VCV folder)."
	echo "You can close this window now."
	exit "$2"
}

# 1. Tools
if ! xcode-select -p >/dev/null 2>&1; then
	finish "FAILED: Xcode Command Line Tools are not installed." 1
fi
if ! command -v make >/dev/null 2>&1; then
	finish "FAILED: 'make' was not found." 1
fi
if [ ! -f "$SDK/plugin.mk" ]; then
	finish "FAILED: Rack-SDK was not found next to this folder (looked in $SDK)." 1
fi
echo "Compiler: $(c++ --version | head -1)"
echo "Machine:  $(uname -m)"

# 2. jq (plugin.mk needs it). Use a tiny stand-in if it is missing.
if ! command -v jq >/dev/null 2>&1; then
	echo "jq not found: using a small stand-in for it."
	SHIM="$(mktemp -d)"
	cat > "$SHIM/jq" <<'JQ'
#!/bin/bash
# Handles only: jq -r .slug plugin.json   and   jq -r .version plugin.json
key="${2#.}"
sed -n "s/.*\"$key\"[[:space:]]*:[[:space:]]*\"\([^\"]*\)\".*/\1/p" "$3" | head -1
JQ
	chmod +x "$SHIM/jq"
	export PATH="$SHIM:$PATH"
fi

# 3. Logic tests
echo
echo "--- Step 1 of 3: automatic logic tests ---"
if bash tests/run_tests.sh; then
	TESTS="passed"
else
	TESTS="FAILED (see above)"
fi
echo "Logic tests: $TESTS"

# 4. Build
echo
echo "--- Step 2 of 3: building the plugin ---"
# The SDK build files cannot cope with spaces in paths, so build through a space-free shortcut.
LINK="$HOME/.hemiola-rack-sdk"
rm -f "$LINK"
ln -s "$(cd "$SDK" && pwd)" "$LINK"
export RACK_DIR="$LINK"
make clean >/dev/null 2>&1
if ! make -j4 dist; then
	# Without zstd the last packaging step fails, but the plugin folder itself is complete.
	if ! command -v zstd >/dev/null 2>&1 && [ -f "dist/HemiolaAudio/plugin.dylib" ]; then
		echo "zstd not found: installing the unpacked plugin folder instead (Rack accepts this)."
		NOZSTD=1
	else
		finish "FAILED: the build stopped with an error. Please tell Claude: the log is ready." 1
	fi
fi

# 5. Install
echo
echo "--- Step 3 of 3: installing into Rack ---"
case "$(uname -m)" in
	arm64) PDIR="$PLUGINS/plugins-mac-arm64" ;;
	*) PDIR="$PLUGINS/plugins-mac-x64" ;;
esac
mkdir -p "$PDIR"
if [ -n "$NOZSTD" ]; then
	rm -rf "$PDIR/HemiolaAudio"
	cp -R dist/HemiolaAudio "$PDIR/"
else
	rm -rf "$PDIR/HemiolaAudio"
	cp dist/*.vcvplugin "$PDIR/"
fi
echo "Installed into: $PDIR"
ls -la "$PDIR" | grep -i hemiola

if pgrep -x Rack >/dev/null 2>&1; then
	finish "DONE (logic tests: $TESTS). Rack is open: quit Rack fully (Cmd+Q) and open it again to load the new modules." 0
fi
finish "DONE (logic tests: $TESTS). Open Rack and look for Hemiola Audio in the module browser." 0
