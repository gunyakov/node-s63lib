# node-s63 API

Exports from `require('node-s63')`.

## Enums

### S63Error

Numeric error constants from s63lib:

- `S63_ERR_OK`
- `S63_ERR_FILE`
- `S63_ERR_DATA`
- `S63_ERR_PERMIT`
- `S63_ERR_KEY`
- `S63_ERR_ZIP`
- `S63_ERR_CRC`

## Static functions (S63 wrappers)

### validateCellPermit(permit, hwId)

- Signature: `validateCellPermit(string, string): boolean`
- Wraps: `S63::validateCellPermit`

### createUserPermit(mKey, hwId, mId)

- Signature: `createUserPermit(string, string, string): string`
- Wraps: `S63::createUserPermit`

### extractHwIdFromUserpermit(userPermit, mKey)

- Signature: `extractHwIdFromUserpermit(string, string): string`
- Wraps: `S63::extractHwIdFromUserpermit`

### createCellPermit(hwId, ck1, ck2, cellName, expiryDate)

- Signature: `createCellPermit(string, Buffer|string, Buffer|string, string, string): string`
- Wraps: `S63::createCellPermit`
- `ck1` and `ck2` are binary 5-byte keys. Use `Buffer` in JS to avoid encoding issues.

### extractCellKeysFromCellpermit(cellPermit, hwId)

- Signature: `extractCellKeysFromCellpermit(string, string): { ok: boolean, key1: Buffer, key2: Buffer }`
- Wraps: `S63::extractCellKeysFromCellpermit`

### decryptCellFromPath(path, key1, key2)

- Signature: `decryptCellFromPath(string, Buffer|string, Buffer|string): { error: number, data: Buffer|null }`
- Wraps: `S63::decryptCell(path, {key1, key2}, outBuf)`
- On success, `data` contains decrypted ZIP bytes.

### decryptCellFromPathAsync(path, key1, key2)

- Signature: `decryptCellFromPathAsync(string, Buffer|string, Buffer|string): Promise<{ error: number, data: Buffer|null }>`
- Async Promise version of `decryptCellFromPath`.

### decryptCellBuffer(data, key)

- Signature: `decryptCellBuffer(Buffer|string, Buffer|string): { error: number, data: Buffer|null }`
- Wraps: `S63::decryptCell(buf, key)`
- On success, `data` is the decrypted buffer.

### decryptCellBufferAsync(data, key)

- Signature: `decryptCellBufferAsync(Buffer|string, Buffer|string): Promise<{ error: number, data: Buffer|null }>`
- Async Promise version of `decryptCellBuffer`.

### encryptCellBuffer(data, key)

- Signature: `encryptCellBuffer(Buffer|string, Buffer|string): Buffer`
- Wraps: `S63::encryptCell`

### decryptAndUnzipCellByKey(inPath, key1, key2, outPath)

- Signature: `decryptAndUnzipCellByKey(string, Buffer|string, Buffer|string, string): number`
- Wraps: `S63::decryptAndUnzipCellByKey`
- Returns `S63Error` code.

### decryptAndUnzipCellByKeyAsync(inPath, key1, key2, outPath)

- Signature: `decryptAndUnzipCellByKeyAsync(string, Buffer|string, Buffer|string, string): Promise<number>`
- Async Promise version of `decryptAndUnzipCellByKey`.
- Resolves to `S63Error` code.

## Class S63Client

JavaScript wrapper over `S63Client` from s63lib.

### new S63Client(hwId, mKey, mId)

- Signature: `new S63Client(string, string, string)`

### getHWID()

- Signature: `getHWID(): string`

### getMID()

- Signature: `getMID(): string`

### getMKEY()

- Signature: `getMKEY(): string`

### setHWID(hwId)

- Signature: `setHWID(string): void`

### setMID(mId)

- Signature: `setMID(string): void`

### setMKEY(mKey)

- Signature: `setMKEY(string): void`

### installCellPermit(cellPermit)

- Signature: `installCellPermit(string): boolean`

### importPermitFile(path)

- Signature: `importPermitFile(string): boolean`

### importPermitFileAsync(path)

- Signature: `importPermitFileAsync(string): Promise<boolean>`
- Async Promise version of `importPermitFile`.

### getUserpermit()

- Signature: `getUserpermit(): string`

### open(path)

- Signature: `open(string): Buffer`
- Returns unzipped S57 cell bytes.
- Empty `Buffer` means operation failed in underlying library.

### openAsync(path)

- Signature: `openAsync(string): Promise<Buffer>`
- Async Promise version of `open`.
- Resolves to unzipped S57 bytes.

### decryptAndUnzipCell(inPath, outPath)

- Signature: `decryptAndUnzipCell(string, string): number`
- Returns `S63Error` code.

### decryptAndUnzipCellAsync(inPath, outPath)

- Signature: `decryptAndUnzipCellAsync(string, string): Promise<number>`
- Async Promise version of `decryptAndUnzipCell`.
- Resolves to `S63Error` code.

### decryptAndUnzipCellWithPermit(inPath, cellPermit, outPath)

- Signature: `decryptAndUnzipCellWithPermit(string, string, string): number`
- Returns `S63Error` code.

### decryptAndUnzipCellWithPermitAsync(inPath, cellPermit, outPath)

- Signature: `decryptAndUnzipCellWithPermitAsync(string, string, string): Promise<number>`
- Async Promise version of `decryptAndUnzipCellWithPermit`.
- Resolves to `S63Error` code.

## Notes on binary inputs

When an argument represents raw key/data bytes, pass `Buffer`:

```js
const key = Buffer.from("C1CB518E9C", "hex");
```

String arguments are treated as UTF-8 bytes.
