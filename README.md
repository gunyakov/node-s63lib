# node-s63

Node.js N-API wrapper for [s63lib](https://github.com/pavelpasha/s63lib).

This package exposes the full public API from `s63.h` and `s63client.h` to JavaScript.

## s63lib as git link (submodule)

This repository tracks `s63lib` as a Git submodule, so the library is linked by commit, not vendored by file copy.

Clone with submodules:

```bash
git clone --recurse-submodules https://githib.com/gunyakov/node-s63lib.git
```

If you already cloned:

```bash
git submodule update --init --recursive
```

Update `s63lib` pointer to a newer upstream commit:

```bash
cd s63lib
git fetch origin
git checkout <commit-or-tag>
cd ..
git add s63lib
git commit -m "Update s63lib submodule"
```

## Install

`npm install node-s63lib` will try to download a precompiled native binary from GitHub Releases first.

If no matching binary exists for your platform/architecture, it falls back to local `node-gyp` compilation.

## Build locally

Requirements:

1. Node.js 18+
2. Build toolchain for `node-gyp` (gcc/g++, make, python)

Install and compile (fallback mode):

```bash
npm install
```

Manual rebuild:

```bash
npm run rebuild
```

## GitHub prebuild workflow

Precompiled binaries are built by GitHub Actions (not by end users) for:

1. Linux x64
2. Windows x64
3. macOS x64
4. macOS arm64

To publish prebuilds:

```bash
git tag v0.1.1
git push origin v0.1.1
```

This triggers `.github/workflows/prebuild.yml`, which builds native binaries and uploads `prebuilds/*.tar.gz` assets to the tagged GitHub Release.

## Auto-build binaries on every push to main

Every push to `main` triggers `.github/workflows/prebuild-main.yml`.

It builds fresh binaries for Linux x64, Windows x64, macOS x64, and macOS arm64, then publishes them as:

1. Workflow artifacts (`prebuilds-<platform>`) in the Actions run
2. Assets on a rolling prerelease tag `main-latest`

This is useful for continuously downloadable binaries from GitHub between formal version tags.

## Auto bump patch version on main

Every push to `main` also triggers `.github/workflows/bump-version.yml`.

It will:

1. Increment patch version in `package.json` and `package-lock.json`
2. Commit the version update to `main`
3. Create and push a matching tag (`vX.Y.Z`)

Pushing the tag automatically triggers `.github/workflows/prebuild.yml`, so versioned GitHub Release binaries are generated for npm installs.

## Quick usage

```js
const s63 = require("node-s63lib");

// Error enum values exported by addon
console.log(s63.S63Error.S63_ERR_OK); // 0

const userPermit = s63.createUserPermit(
  "98765", // mKey: manufacturer key from your S-63 provider
  "12348", // hwId: your device/system hardware ID
  "01", // mId: manufacturer ID assigned by your provider
);
console.log(userPermit); // e.g. 73871727080876A07E450C043031
```

## Example: extract cell keys and decrypt file

```js
const s63 = require("node-s63lib");

const hwId = "12348";
const cellPermit =
  "NO4D061320000830BEB9BFE3C7C6CE68B16411FD09F96982795C77B204F54D48";

const extracted = s63.extractCellKeysFromCellpermit(cellPermit, hwId);
if (!extracted.ok) {
  throw new Error("Invalid cell permit or HW ID");
}

// key1/key2 are Node.js Buffers
const result = s63.decryptCellFromPath(
  "/charts/NO4D06.000",
  extracted.key1,
  extracted.key2,
);
if (result.error !== s63.S63Error.S63_ERR_OK) {
  throw new Error(`decrypt failed: ${result.error}`);
}

// result.data is decrypted ZIP payload as Buffer
console.log(result.data.length);
```

## Example: decrypt and save directly

```js
const s63 = require("node-s63lib");

const err = s63.decryptAndUnzipCellByKey(
  "/charts/NO4D06.000", // inPath: encrypted S-63 cell file (.000)
  Buffer.from("C1CB518E9C", "hex"), // ck1: first 5-byte cell key from permit
  Buffer.from("421571CC66", "hex"), // ck2: second 5-byte cell key from permit
  "/out/NO4D06.000", // outPath: destination for decrypted/unzipped S-57 cell
);

if (err !== s63.S63Error.S63_ERR_OK) {
  throw new Error(`decryptAndUnzipCellByKey failed: ${err}`);
}
```

## Example: S63Client workflow

```js
const { S63Client, S63Error } = require("node-s63");

const client = new S63Client(
  "12348", // hwId: your device/system hardware ID
  "98765", // mKey: manufacturer key from your S-63 provider
  "01", // mId: manufacturer ID assigned by your provider
);

const userPermit = client.getUserpermit();
console.log("User permit:", userPermit);

client.importPermitFile("/permits/PERMIT.TXT");

const rawS57 = client.open("/charts/NO4D06.000");
console.log(Buffer.isBuffer(rawS57), rawS57.length);

const err = client.decryptAndUnzipCell("/charts/NO4D06.000", "/out/NO4D06.000");
if (err !== S63Error.S63_ERR_OK) {
  throw new Error(`decryptAndUnzipCell failed: ${err}`);
}
```

## Promise-based async API

For file and decryption operations, Promise versions are available to avoid blocking the event loop.

Top-level async functions:

- `decryptCellFromPathAsync(path, key1, key2)`
- `decryptCellBufferAsync(data, key)`
- `decryptAndUnzipCellByKeyAsync(inPath, key1, key2, outPath)`

S63Client async methods:

- `importPermitFileAsync(path)`
- `openAsync(path)`
- `decryptAndUnzipCellAsync(inPath, outPath)`
- `decryptAndUnzipCellWithPermitAsync(inPath, cellPermit, outPath)`

Example:

```js
const { S63Client, S63Error } = require("node-s63lib");

async function run() {
  const client = new S63Client("12348", "98765", "01");

  const imported = await client.importPermitFileAsync("/permits/PERMIT.TXT");
  if (!imported) throw new Error("Failed to import permit file");

  const data = await client.openAsync("/charts/NO4D06.000");
  console.log("Open bytes:", data.length);

  const err = await client.decryptAndUnzipCellAsync(
    "/charts/NO4D06.000",
    "/out/NO4D06.000",
  );

  if (err !== S63Error.S63_ERR_OK) {
    throw new Error(`decryptAndUnzipCellAsync failed: ${err}`);
  }
}

run().catch((err) => {
  console.error(err);
  process.exit(1);
});
```

## API reference

See [api.md](api.md).
