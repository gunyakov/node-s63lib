declare namespace NodeS63Lib {
  type BinaryInput = Uint8Array | string;

  interface DecryptResult {
    error: number;
    data: Uint8Array | null;
  }

  interface CellKeysResult {
    ok: boolean;
    key1: Uint8Array;
    key2: Uint8Array;
  }

  interface S63ErrorMap {
    readonly S63_ERR_OK: number;
    readonly S63_ERR_FILE: number;
    readonly S63_ERR_DATA: number;
    readonly S63_ERR_PERMIT: number;
    readonly S63_ERR_KEY: number;
    readonly S63_ERR_ZIP: number;
    readonly S63_ERR_CRC: number;
  }

  class S63Client {
    constructor(hwId: string, mKey: string, mId: string);

    getHWID(): string;
    getMID(): string;
    getMKEY(): string;

    setHWID(hwId: string): void;
    setMID(mId: string): void;
    setMKEY(mKey: string): void;

    installCellPermit(cellPermit: string): boolean;

    importPermitFile(path: string): boolean;
    importPermitFileAsync(path: string): Promise<boolean>;

    getUserpermit(): string;

    open(path: string): Uint8Array;
    openAsync(path: string): Promise<Uint8Array>;

    decryptAndUnzipCell(inPath: string, outPath: string): number;
    decryptAndUnzipCellAsync(inPath: string, outPath: string): Promise<number>;

    decryptAndUnzipCellWithPermit(
      inPath: string,
      cellPermit: string,
      outPath: string,
    ): number;
    decryptAndUnzipCellWithPermitAsync(
      inPath: string,
      cellPermit: string,
      outPath: string,
    ): Promise<number>;
  }
}

declare const addon: {
  validateCellPermit(permit: string, hwId: string): boolean;
  createUserPermit(mKey: string, hwId: string, mId: string): string;
  extractHwIdFromUserpermit(userPermit: string, mKey: string): string;

  createCellPermit(
    hwId: string,
    ck1: NodeS63Lib.BinaryInput,
    ck2: NodeS63Lib.BinaryInput,
    cellName: string,
    expiryDate: string,
  ): string;

  extractCellKeysFromCellpermit(
    cellPermit: string,
    hwId: string,
  ): NodeS63Lib.CellKeysResult;

  decryptCellFromPath(
    path: string,
    key1: NodeS63Lib.BinaryInput,
    key2: NodeS63Lib.BinaryInput,
  ): NodeS63Lib.DecryptResult;

  decryptCellFromPathAsync(
    path: string,
    key1: NodeS63Lib.BinaryInput,
    key2: NodeS63Lib.BinaryInput,
  ): Promise<NodeS63Lib.DecryptResult>;

  decryptCellBuffer(
    data: NodeS63Lib.BinaryInput,
    key: NodeS63Lib.BinaryInput,
  ): NodeS63Lib.DecryptResult;
  decryptCellBufferAsync(
    data: NodeS63Lib.BinaryInput,
    key: NodeS63Lib.BinaryInput,
  ): Promise<NodeS63Lib.DecryptResult>;

  encryptCellBuffer(
    data: NodeS63Lib.BinaryInput,
    key: NodeS63Lib.BinaryInput,
  ): Uint8Array;

  decryptAndUnzipCellByKey(
    inPath: string,
    key1: NodeS63Lib.BinaryInput,
    key2: NodeS63Lib.BinaryInput,
    outPath: string,
  ): number;

  decryptAndUnzipCellByKeyAsync(
    inPath: string,
    key1: NodeS63Lib.BinaryInput,
    key2: NodeS63Lib.BinaryInput,
    outPath: string,
  ): Promise<number>;

  S63Error: NodeS63Lib.S63ErrorMap;
  S63Client: typeof NodeS63Lib.S63Client;
};

export = addon;
