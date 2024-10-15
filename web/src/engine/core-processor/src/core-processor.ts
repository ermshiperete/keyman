type km_core_attr = import('./import/core/km-core-interface.js').km_core_attr;

// Unfortunately embind has an open issue with enums and typescript where it
// only generates a type for the enum, but not the values in a usable way.
// So we have to re-define the enum here.
// See https://github.com/emscripten-core/emscripten/issues/18585
// NOTE: Keep in sync with core/include/keyman/keyman_core_api.h#L311
export enum KM_CORE_STATUS {
  OK = 0,
  NO_MEM = 1,
  IO_ERROR = 2,
  INVALID_ARGUMENT = 3,
  KEY_ERROR = 4,
  INSUFFICENT_BUFFER = 5,
  INVALID_UTF = 6,
  INVALID_KEYBOARD = 7,
  NOT_IMPLEMENTED = 8,
  OS_ERROR = 0x80000000
}

export type CoreKeyboard = {};

export class CoreProcessor {
  private instance: any;

  /**
   * Initialize Core Processor
   * @param baseurl - The url where km-core.js is located
   */
  public async init(baseurl: string): Promise<boolean> {

    if (!this.instance) {
      try {
        const module = await import(baseurl + '/km-core.js');
        this.instance = await module.default({
          locateFile: function (path: string, scriptDirectory: string) {
            return baseurl + '/' + path;
          }
        });
      } catch (e: any) {
        console.log('got execption in CoreProcessor.init', e);
        return false;
      }
    }
    return !!this.instance;
  };

  public tmp_wasm_attributes(): km_core_attr {
    return this.instance.tmp_wasm_attributes();
  }

  public km_core_keyboard_load_from_blob(kb_name: string, blob: Uint8Array, keyboard: CoreKeyboard): KM_CORE_STATUS {
    return null;  // this.instance.km_core_keyboard_load_from_blob(kb_name, blob, blob.length, keyboard);
  }
}
