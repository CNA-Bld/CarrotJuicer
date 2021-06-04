# EXNOA-CarrotJuicer

Hooks the decryption function in `libnative.dll` of ウマ娘プリティーダービー (Umamusume Pretty Derby), to allow inspecting the packets (and provide some useful information during the game).

For Android, refer to [Riru-CarrotJuicer](https://github.com/CNA-Bld/Riru-CarrotJuicer).

## Usage

Theoretically this should support "modern" versions of Windows, as long as it is x64. But this is only tested with Windows 10 v2004.

Please make sure that you have installed the latest Visual C++ 2019 Redistributable, otherwise the game would crash at start up time with no message at all.

If the precompiled binary from Releases page does not work, try build it yourself. I have seen at least 2 machines where it works only when building locally. I have no idea why, probably the VS build process (or even installing VS?) changes some state in the system.

1. Copy `version.dll` to the same directory with `umamusume.exe`. This should be `%USERPROFILE%\Umamusume` unless you did some magic yourself.
2. Start the game as usual (i.e., with DMM launcher). The captured packets will be saved into `CarrotJuicer` folder in the game directory.
3. You can investigate the responses with msgpack tools like `msgpack2json -di 123456789R.msgpack`.

[Hakuraku](https://github.com/SSHZ-ORG/hakuraku) has a UI for investigating the captured packets [here](https://hakuraku.sshz.org/#/carrotjuicer).

### `cjedb.json` and `master.mdb`

Starting from v1.2, EXNOA-CarrotJuicer would print extra info that may help users to make strategic decisions. Some features depend on an optional external data file `cjedb.json`. If this file is missing, some features will be disabled.

The Releases in this repo would bundle the latest file as of that time, but you may wish to check for updates [here](https://github.com/CNA-Bld/cjedb) from time to time, especially after a new charactor or support card is added.

In addition, EXNOA-CarrotJuicer will attempt to read `master.mdb` directly from the game's data directory (in `%USERPROFILE%\AppData\LocalLow\Cygames\umamusume\master`) with a bundled SQLite engine. (Sorry for the bloating file size, but the game itself takes 4GB anyway, so we are as trivial as some rounding error.) If you somehow moved it, please at least make sure a link is available.

### `race_scenario`

In packets containing races, there is a base64-encoded field, often named `race_scenario`. This includes per-frame and per-chara information in the race.

More details can be found in README of [Hakuraku](https://github.com/SSHZ-ORG/hakuraku). Its web UI is able to parse this field for you.

### Requests

Requests (files ending with `Q.msgpack`) are not actually msgpack. The current observation is:

* The first 4 bytes likely represent a little-endian int. We name it `offset`, currently always observed to be 0xA6 (166).
* The following 52 bytes `[+0x04, +0x38)` never change for a single client, even across sessions. We did not test whether this is per-account or per-client.
* The following 114 bytes `[+0x38, +0xB0)` are different for each request.
* All remaining is a standard msgpack message. This starts at `+0xB0` which is exactly `offset + 4`.

To investigate the content, remove the first 170 bytes and use msgpack tools, like `tail -c+171 123456789Q.msgpack | msgpack2json -d`.

## Build

0. Install [vcpkg](https://vcpkg.io/en/getting-started.html), and make sure to enable VS integration by running `vcpkg integrate install`.
1. `git clone`
2. Spin up Visual Studio 2019, and press "Build".

## Credits

This module is largely ~~copied from~~ inspired by [umamusume-localify](https://github.com/GEEKiDoS/umamusume-localify).
