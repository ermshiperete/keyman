import { MainModule as KmCoreModule, KM_CORE_STATUS, km_core_keyboard } from 'keyman/engine/core-processor';
import Keyboard from "./keyboard.js";
import { KeyboardHarness } from "./keyboardHarness.js";
import KeyboardProperties from "./keyboardProperties.js";
import { KeyboardLoadErrorBuilder, StubBasedErrorBuilder, UriBasedErrorBuilder } from './keyboardLoadError.js';

export type KeyboardStub = KeyboardProperties & { filename: string };

export abstract class KeyboardLoaderBase {
  private _harness: KeyboardHarness;
  protected _km_core: KmCoreModule;

  public get harness(): KeyboardHarness {
    return this._harness;
  }

  constructor(harness: KeyboardHarness, km_core?: KmCoreModule) {
    this._harness = harness;
    this._km_core = km_core;
  }

  /**
   * Load a keyboard from a remote or local URI.
   *
   * @param uri  The URI of the keyboard to load.
   * @returns    A Promise that resolves to the loaded keyboard.
   */
  public loadKeyboardFromPath(uri: string): Promise<Keyboard|km_core_keyboard> {
    this.harness.install();
    return this.loadKeyboardInternal(uri, new UriBasedErrorBuilder(uri));
  }

  /**
   * Load a keyboard from keyboard stub.
   *
   * @param stub  The stub of the keyboard to load.
   * @returns     A Promise that resolves to the loaded keyboard.
   */
  public async loadKeyboardFromStub(stub: KeyboardStub): Promise<Keyboard|km_core_keyboard> {
    this.harness.install();
    return this.loadKeyboardInternal(stub.filename, new StubBasedErrorBuilder(stub));
  }

  private async loadKeyboardInternal(uri: string, errorBuilder: KeyboardLoadErrorBuilder): Promise<Keyboard|km_core_keyboard> {
    const byteArray = await this.loadKeyboardBlob(uri, errorBuilder);

    if (byteArray.slice(0, 4) == Uint8Array.from([0x4b, 0x58, 0x54, 0x53])) { // 'KXTS'
      // KMX or LDML (KMX+) keyboard
      const result = this._km_core.keyboard_load_from_blob(uri, byteArray);
      if (result.status == KM_CORE_STATUS.OK) {
        return result.object;
      }
      return null;
    }

    let script: string;
    try {
      script = new TextDecoder('utf-8', { fatal: true }).decode(byteArray);
    } catch (e) {
      throw errorBuilder.invalidKeyboard(e);
    }

    // .js keyboard
    return await this.loadKeyboardFromScript(script, errorBuilder);
  }

  protected abstract loadKeyboardBlob(uri: string, errorBuilder: KeyboardLoadErrorBuilder): Promise<Uint8Array>;

  protected abstract loadKeyboardFromScript(scriptSrc: string, errorBuilder: KeyboardLoadErrorBuilder): Promise<Keyboard>;
}